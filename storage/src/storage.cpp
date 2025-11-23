#include "storage.hpp"

#include <fstream>
#include <optional>
#include <regex>
#include <sstream>

namespace localpm::file_process {

using semver::semver_exception;
using semver::version;

// --------- helpers ---------

bool is_valid_ident(std::string_view s) {
	static const std::regex re(R"([a-z0-9][a-z0-9._-]{0,63})");
	return std::regex_match(s.begin(), s.end(), re);
}

// обёртка над semver::version::parse, чтобы кидать нашу ошибку
static version parse_version_or_throw(std::string_view vstr) {
	try {
		return version::parse(std::string(vstr)); // strict по умолчанию
	} catch (const semver_exception &e) {
		throw SemVerParseError(std::string(vstr), e.what());
	}
}

static std::string normalize_version_string(const version &v) {
	std::ostringstream oss;
	oss << v; // cpp-semver печатает канонизированный вид
	return oss.str();
}

// --------- StorageLayout ---------

StorageLayout::StorageLayout(fs::path root_) : root(std::move(root_)) {
	// можно сразу нормализовать в absolute, если хочешь
	// root = fs::absolute(root);

	config = root / "config.toml";
	logs = root / "logs";
	cache = root / "cache";
	index_dir = root / "index";
	index_db = root / "index.db"; // sqlite-файл (по твоей схеме index.db)
	packages = root / "packages";
}

// --------- PackageLayout ---------

PackageLayout::PackageLayout(const StorageLayout &sl, std::string_view ns,
							 std::string_view name, std::string_view version) {
	ns_dir = sl.packages / std::string(ns);
	pkg_dir =
		ns_dir / std::string(name); // ВАЖНО: ns/name, а не sl.packages/name
	ver_dir = pkg_dir / std::string(version);
	manifest = ver_dir / "manifest.toml";
	source_dir = ver_dir / "source";
	build_dir = ver_dir / "build";
	meta_json = ver_dir / "meta.json";
	latest_link = pkg_dir / "latest";
}

// --------- ensure_dir ---------

void ensure_dir(const fs::path &path) {
	if (fs::exists(path)) {
		if (!fs::is_directory(path)) {
			throw NotDirError(path.string());
		}
		return;
	}

	fs::create_directories(path);
}

// --------- init_storage ---------

void init_storage(const fs::path &root_) {
	StorageLayout sl(root_);

	ensure_dir(sl.root);
	ensure_dir(sl.logs);
	ensure_dir(sl.cache);
	ensure_dir(sl.index_dir);
	ensure_dir(sl.packages);

	// простой дефолтный config, если его нет
	if (!fs::exists(sl.config)) {
		std::ofstream cfg(sl.config);
		if (cfg) {
			cfg << "# LocalPM config\n"
				<< "store_path = \"" << sl.root.string() << "\"\n";
		}
	}

	// пока просто гарантируем наличие файла index.db
	if (!fs::exists(sl.index_db)) {
		std::ofstream db(sl.index_db);
		// реальная схема создаётся через sqlite3 / SQLiteCpp
	}
}

// --------- update_latest_symlink ---------

void update_latest_symlink(const PackageLayout &pl, bool stable_only) {
	std::optional<version> best;
	fs::path best_path;

	if (!fs::exists(pl.pkg_dir) || !fs::is_directory(pl.pkg_dir)) {
		// пакета вообще нет — можно просто удалить latest (если был)
		if (fs::exists(pl.latest_link) || fs::is_symlink(pl.latest_link)) {
			fs::remove(pl.latest_link);
		}
		return;
	}

	for (const auto &entry : fs::directory_iterator(pl.pkg_dir)) {
		if (!entry.is_directory())
			continue;

		const auto dir_name = entry.path().filename().string();
		if (dir_name == "latest")
			continue; // симлинк самого latest

		try {
			auto v = version::parse(dir_name);
			if (stable_only && !v.is_stable()) {
				continue;
			}

			if (!best || *best < v) {
				best = v;
				best_path = entry.path();
			}
		} catch (const semver_exception &) {
			// невалидное имя каталога — игнорируем
			continue;
		}
	}

	if (!best) {
		// нет ни одной подходящей версии — удаляем latest
		if (fs::exists(pl.latest_link) || fs::is_symlink(pl.latest_link)) {
			fs::remove(pl.latest_link);
		}
		return;
	}

	// обновляем симлинк
	if (fs::exists(pl.latest_link) || fs::is_symlink(pl.latest_link)) {
		fs::remove(pl.latest_link);
	}

	// делаем относительный симлинк latest -> <best_version>/
	fs::create_directory_symlink(best_path.filename(), pl.latest_link);
}

// --------- ensure_package_version ---------

void ensure_package_version(const StorageLayout &sl, std::string_view ns,
							std::string_view name, std::string_view version_str,
							std::string_view manifest_content) {
	if (!is_valid_ident(ns)) {
		throw std::invalid_argument("Invalid namespace: " + std::string(ns));
	}
	if (!is_valid_ident(name)) {
		throw std::invalid_argument("Invalid package name: " +
									std::string(name));
	}

	// парсим SemVer и нормализуем
	version ver = parse_version_or_throw(version_str);
	const std::string normalized_version = normalize_version_string(ver);

	PackageLayout pl(sl, ns, name, normalized_version);

	ensure_dir(pl.ns_dir);
	ensure_dir(pl.pkg_dir);
	ensure_dir(pl.ver_dir);
	ensure_dir(pl.source_dir);
	ensure_dir(pl.build_dir);

	if (fs::exists(pl.manifest)) {
		throw PackageVersionExistsError(std::string(ns), std::string(name),
										normalized_version);
	}

	// пишем manifest.toml
	{
		std::ofstream mf(pl.manifest);
		if (!mf) {
			throw std::runtime_error("Failed to open manifest for write: " +
									 pl.manifest.string());
		}
		mf << manifest_content;
	}

	// тут же можно записать meta_json и обновить index.db

	// пересчитываем latest после успешной установки версии
	update_latest_symlink(pl);
}

void import_package_version(const StorageLayout &sl, std::string_view ns,
							std::string_view name, const fs::path &src_ver_dir,
							std::string version_str) {
	if (!fs::exists(src_ver_dir)) {
		throw std::invalid_argument(
			"Source directory does not exist or is not a directory: " +
			src_ver_dir.string());
	}

	if (!is_valid_ident(ns)) {
		throw std::invalid_argument("Invalid package namespace: " +
									std::string(ns));
	}

	if (!is_valid_ident(name)) {
		throw std::invalid_argument("Invalid package name: " +
									std::string(name));
	}

	if (version_str.empty()) {
		version_str = src_ver_dir.filename().string();
	}

	version ver = parse_version_or_throw(version_str);
	const std::string normalized_version = normalize_version_string(ver);

	PackageLayout pl(sl, ns, name, normalized_version);

	// Создаём namespace и пакет при необходимости
	ensure_dir(pl.ns_dir);
	ensure_dir(pl.pkg_dir);

	// Если такая версия уже есть — кидаем твою специальную ошибку
	if (fs::exists(pl.ver_dir)) {
		throw PackageVersionExistsError(std::string(ns), std::string(name),
										normalized_version);
	}

	// Создаём каталог версии
	ensure_dir(pl.ver_dir);

	// 4. Копируем содержимое src_ver_dir в ver_dir
	//    (именно contents, а не сам каталог как подкаталог)
	for (const auto &entry : fs::directory_iterator(src_ver_dir)) {
		const fs::path &from = entry.path();
		fs::path to = pl.ver_dir / from.filename();

		if (entry.is_directory()) {
			fs::copy(from, to,
					 fs::copy_options::recursive |
						 fs::copy_options::copy_symlinks);
		} else if (entry.is_regular_file()) {
			fs::copy(from, to,
					 fs::copy_options::copy_symlinks |
						 fs::copy_options::overwrite_existing);
		} else {
			// спецфайлы можно игнорировать или обработать отдельно
		}
	}

	// 5. На всякий случай проверим наличие manifest.toml
	if (!fs::exists(pl.manifest)) {
		throw std::runtime_error(
			"Imported version directory does not contain manifest.toml: " +
			pl.manifest.string());
	}

	// 6. Обновляем latest для этого пакета
	update_latest_symlink(pl);
}

} // namespace localpm::file_process
