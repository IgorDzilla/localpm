#include "manifest/manifest.hpp"

#include <fstream>
#include <stdexcept>
#include <toml++/toml.hpp>

namespace localpm::manifest {
using ::toml::array;
using ::toml::table;

// ---------------------------------------------------------
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ СТРОК ИЗ TOML
// ---------------------------------------------------------

// give string from table, if key not found - exception
static std::string get_required_string(const table &tbl, std::string_view key) {
	if (auto value = tbl[key].value<std::string>()) {
		return *value;
	}
	throw std::runtime_error{"manifest.toml: missing required string key ' " +
							 std::string(key) + "'"};
}

// give optional string from table< if not found - std::nulopt
static std::optional<std::string> get_optional_string(const table &tbl,
													  std::string_view key) {
	if (auto value = tbl[key].value<std::string>()) {
		return *value;
	}
	return std::nullopt;
}

// ---------------------------------------------------------
// РАЗБОР ОДНОЙ ЗАВИСИМОСТИ ИЗ TOML-УЗЛА
// ---------------------------------------------------------

// Эта функция берёт имя зависимости (ключ в [dependencies])
// и узел TOML (значение) и собирает структуру DependencyDecl.
//
// Поддерживаются две формы:
//   1) короткая:  foo = ">=1.2"
//   2) полная:    foo = { version = "...", git = "...", ... }

static DependencyDecl parse_single_dependency(const std::string &name,
											  const toml::node &node) {
	DependencyDecl dep{};
	dep.name = name;

	// case 1 - short form, value = string
	if (auto v = val.as_value()) {
		if (auto s = v->value<std::string>()) {
			dep.vesion_spec = *s;
		}
		// other field leave in empty (std::nullopt)
		return dep;
	}

	// case 2 - long form, value = table
	if (auto tbl = node.as_table()) {

		// version - optional
			  if (auto v = tbl->operator[]("version").value<std::string>{
			dep.version_spec = *v;
              }

              //other field - optional
              if (auto v = tbl->operator[]("registry").value<std::string>()) {
			dep.registry = *v;
              }
              if (auto v = tbl->operator[]("git").value<std::string>()) {
			dep.git = *v;
              }
              if (auto v = tbl->operator[]("tag").value<std::string>()) {
			dep.tag = *v;
              }
              if (auto v = tbl->operator[]("rev").value<std::string>()) {
			dep.rev = *v;
              }
              if (auto v = tbl->operator[]("path").value<std::string>()) {
			dep.path = *v;
              }
              if (auto v = tbl->operator[]("archive").value<std::string>()) {
			dep.archive = *v;
              }
              if (auto v = tbl->operator[]("checksum").value<std::string>()) {
			dep.checksum = *v;
              }
	}
}

// ---------------------------------------------------------
// РАЗБОР ЦЕЛОЙ СЕКЦИИ ЗАВИСИМОСТЕЙ
// ---------------------------------------------------------

// Эта функция берёт таблицу секции ([dependencies] и т.п.)
// и заполняет вектор зависимостей.
//
// Если таблицы нет (nullptr) — просто ничего не делает.
static void parse_dependency_section(const table *deps_tbl,
									 std::vector<DependencyDecl> &out) {
	if (!deps_tbl) {
		return; // section is empty
	}

	// walk at all pairs key = value in table
	for (auto &&[key, node] : *deps_tbl) {
		DependencyDecl dep = parse_single_dependency(key.str(), node);
		out.push_back(std::move(dep));
	}
}

// ---------------------------------------------------------
// СОЗДАНИЕ TOML-УЗЛА ДЛЯ ОДНОЙ ЗАВИСИМОСТИ
// ---------------------------------------------------------

// Здесь мы решаем, как записать зависимость обратно в TOML:
//   - если только версия — короткая форма:  foo = "1.2.3"
//   - если есть доп. поля (git/path/…) — полная форма: foo = { ... }
static toml::node make_single_node(const DependencyDecl &d) {
	const bool only_version = !d.version_spec.empty() && !d.registry &&
							  !d.git && !d.tag && !d.rev && !d.path &&
							  !d.archive && !d.checksum;

	if (only_version) {
		// short write foo = "1.2.3"
		return toml::node{d.version_spec};
	}

	// full frite: foo = { version = "...", git = "...", ... }
	table tbl;

	if (!d.version_spec.empty()) {
		tbl.insert("version", d.version_spec);
	}
	if (d.registry) {
		tbl.insert("registry", *d.registry);
	}
	if (d.git) {
		tbl.insert("git", *d.git);
	}
	if (d.tag) {
		tbl.insert("tag", *d.tag);
	}
	if (d.rev) {
		tbl.insert("rev", *d.rev);
	}
	if (d.path) {
		tbl.insert("path", *d.path);
	}
	if (d.archive) {
		tbl.insert("archive", *d.archive);
	}
	if (d.checksum) {
		tbl.insert("checksum", *d.checksum);
	}

	return toml::node{std::move(tbl)};
}

// ---------------------------------------------------------
// СОЗДАНИЕ ЦЕЛОЙ СЕКЦИИ ЗАВИСИМОСТЕЙ
// ---------------------------------------------------------
static table make_dependency_section(const std::vector<DependencyDecl> &deps) {
	table deps_tbl;

	for (const auto &d : deps) {
		deps_tbl.insert(d.name, make_single_dependency_node(d));
	}

	return deps_tbl;
}

// ---------------------------------------------------------
// ОСНОВНАЯ ЛОГИКА: TOML -> Manifest
// ---------------------------------------------------------
Manifest ManifestTomlAdapter::from_toml(const toml::table &root) {
	Manifest m{};

	// ---- [package] ----
	const table *pkg_tbl = root["package"].as_table();
	if (!pkg_tbl) {
		throw std::runtime_error{"manifest.toml : missing [package] section"};
	}

	// read important tiles
	m.package.name = get_required_string(*pkg_tbl, "name");
	m.package.version = get_required_string(*pkg_tbl, "version");

	// read optional tiles
	m.package.description = get_optional_string(*pkg_tbl, "description");
	m.package.license = get_optional_string(*pkg_tbl, "license");
	m.package.homepage = get_optional_string(*pkg_tbl, "homepage");
	m.package.kind = get_optional_string(*pkg_tbl, "kind");

	// authors = ["...", "..."]
	if (auto authors_arr = (*pkg_tbl)["authors"].as_array()) {
		for (auto &&node : *authors_arr) {
			if (auto s = node.value<std::string>()) {
				m.package.authors.push_back(*s);
			}
		}
	}

	// ---------- [dependencies] ----------
	parse_dependency_section(root["dependencies"].as_table(), m.dependencies);

	// ---------- [dev-dependencies] ----------
	parse_dependency_section(root["dev-dependencies"].as_table(),
							 m.dev_dependencies);

	// ---------- [build-dependencies] ----------
	parse_dependency_section(root["build-dependencies"].as_table(),
							 m.build_dependencies);

	return m;
}

// ---------------------------------------------------------
// ОСНОВНАЯ ЛОГИКА: Manifest -> TOML
// ---------------------------------------------------------

toml::table ManifestTomlAdapter::to_toml(const Manifest &m) {
	table root;

	// ---------- [package] ----------
	table pkg;

	pkg.insert("name", m.package.name);
	pkg.insert("version", m.package.version);

	if (m.package.description) {
		pkg.insert("description", *m.package.description);
	}
	if (m.package.license) {
		pkg.insert("license", *m.package.license);
	}
	if (m.package.homepage) {
		pkg.insert("homepage", *m.package.homepage);
	}
	if (m.package.kind) {
		pkg.insert("kind", *m.package.kind);
	}

	if (!m.package.authors.empty()) {
		array authors;
		for (const auto &a : m.package.authors) {
			authors.push_back(a);
		}
		pkg.insert("authors", std::move(authors));
	}

	root.insert("package", std::move(pkg));

	// ---------- [dependencies] ----------
	if (!m.dependencies.empty()) {
		root.insert("dependencies", make_dependency_section(m.dependencies));
	}

	if (!m.dev_dependencies.empty()) {
		root.insert("dev-dependencies",
					make_dependency_section(m.dev_dependencies));
	}

	if (!m.build_dependencies.empty()) {
		root.insert("build-dependencies",
					make_dependency_section(m.build_dependencies));
	}

	return root;
}

} // namespace localpm::manifest

// =============================================================
//  ЧАСТЬ, КОТОРАЯ РАБОТАЕТ С ФАЙЛАМИ НА ДИСКЕ
// =============================================================
namespace localpm::filesys {
using localpm::manifest::Manifest;
using localpm::manifest::ManifestTomlAdapter;

ManifestProcessor::ManifestProcessor(const std::filesystem::path &path)
	: path_(path) {
	reload();
}

// rewrite ,anifest.toml from disk
void ManifestProcessor::reload() {
	toml::parse_result parsed = toml::parse_file(path_.string());
	const toml::table &root = parsed.table();

	manifest_ = ManifestTomlAdapter::from_toml(root);
}

// save current Manifest in manifest.toml
void ManifestProcessor::save() const {
	toml::table root = ManifestTomlAdapter::to_toml(manifest_);

	std::ofstream os(path_);
	if (!os) {
		throw std::runtime_error{"Failed to open manifest file writting: " +
								 path_.string()};
	}

	os << root;
}

// free function
Manifest load_manifest(const std::filesystem::path &path) {
	ManifestProcessor p{path};
	return p.data();
}

void write_manifest(const Manifest &m, const std::filesystem::path &path) {
	ManifestProcessor p{path};
	p.data() = m;
	p.save();
}

} // namespace localpm::filesys
