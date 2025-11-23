#include "database.hpp"
#include "logger/logger.h"
#include <SQLiteCpp/Statement.h>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <semver/semver.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace localpm::database{

// --- util ---
static std::string assemble_path(std::string path, std::string filename) {
	if (!path.empty() && path.back() != '/') {
		return path + "/" + filename;
	}

	return path + filename;
}

DataBase::DataBase(std::string &path_par)
	: path(path_par), db((std::filesystem::create_directories(
							  std::filesystem::path(path_par)
								  .parent_path()), // гарантируем каталог
						  path_par),
						 SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {}

void DataBase::init_db() {
	std::string query_path =
		assemble_path(std::string(DB_QUERY_FOLDER), std::string(INIT_QUERY));
	std::fstream input(query_path);

	std::cerr << query_path << std::endl;

	if (!input) {
		throw DataBaseError("Initialization query file not found",
							DataBaseErrorCode::QUERY_FILE_NOT_FOUND);
	}

	std::stringstream buffer;
	buffer << input.rdbuf();
	try {
		db.exec(buffer.str());
		db.exec("PRAGMA foreign_keys=ON;");
	} catch (std::exception &e) {
		std::string err_str =
			std::string("Query execution failed: ") + std::string(e.what());
		throw DataBaseError(err_str, DataBaseErrorCode::QUERY_FAILURE);
	}
}

std::vector<Package>
DataBase::search_package_versions(std::string ns, std::string name,
								  std::string min_ver_str) {
	SQLite::Statement query(db,
							"SELECT "
							"  id, name, namespace, version, path, "
							"  source_type, pkg_type, created_at, updated_at "
							"FROM packages "
							"WHERE namespace = ? AND name = ? AND deleted = 0");

	query.bind(1, ns);
	query.bind(2, name);

	std::vector<std::pair<semver::version, Package>> tmp_pkgs;
	std::optional<semver::version> min_ver = std::nullopt;

	if (!min_ver_str.empty()) {
		min_ver = semver::version::parse(min_ver_str);
	}

	while (query.executeStep()) {
		const std::string ver_str = query.getColumn("version").getString();

		semver::version ver;
		try {
			ver = semver::version::parse(ver_str);
		} catch (const std::exception &e) {
			LOG_WARN(std::string("Incorrect version \"") + ver_str +
					 std::string("\" discovered in database"));
			continue;
		}

		if (min_ver.has_value()) {
			if (ver < min_ver.value()) {
				continue;
			}
		}

		Package p;
		// clang-format off
		p.id          	= query.getColumn("id").getInt();
        p.name        	= query.getColumn("name").getString();
        p.pkg_namespace = query.getColumn("namespace").getString();
        p.version       = ver_str;
        p.path        	= query.getColumn("path").getString();
        p.src_type    	= query.getColumn("source_type").getString();
        p.pkg_type    	= query.getColumn("pkg_type").getString();
        p.created_at  	= query.getColumn("created_at").getInt64();
        p.updated_at  	= query.getColumn("updated_at").getInt64();
		// clang-format on

		tmp_pkgs.emplace_back(std::move(ver), std::move(p));
	}

	std::sort(tmp_pkgs.begin(), tmp_pkgs.end(),
			  [](const auto &a, const auto &b) {
				  return a.first > b.first; // убывание
			  });

	std::vector<Package> result;
	result.reserve(tmp_pkgs.size());

	for (std::size_t i = 0; i < tmp_pkgs.size(); i++) {
		result.emplace_back(std::move(tmp_pkgs[i].second));
	}

	return result;
}

std::unordered_map<std::string, Package>
DataBase::search_packages(std::vector<std::string> namespaces,
						  std::vector<std::string> names,
						  std::string min_version) {
	std::string query_str = "SELECT "
							"  id, name, namespace, version, path, "
							"  source_type, pkg_type, created_at, updated_at "
							"FROM packages "
							"WHERE deleted = 0";
	if (!namespaces.empty()) {
		query_str += " AND namespace IN (";
		for (std::size_t i = 0; i < namespaces.size(); i++) {
			if (i) {
				query_str += ',';
			}
			query_str += '?';
		}
		query_str += ')';
	}

	if (!names.empty()) {
		query_str += " AND name IN (";
		for (std::size_t i = 0; i < names.size(); i++) {
			if (i) {
				query_str += ',';
			}
			query_str += '?';
		}
		query_str += ')';
	}

	SQLite::Statement stmt(db, query_str);

	int bind_index = 1;
	for (const auto &ns : namespaces) {
		stmt.bind(bind_index++, ns);
	}

	for (const auto &n : names) {
		stmt.bind(bind_index++, n);
	}

	semver::version min_ver;
	bool has_min_ver = false;
	if (!min_version.empty()) {
		try {
			min_ver = semver::version::parse(min_version);
			has_min_ver = true;
		} catch (const std::exception &) {
			LOG_WARN(std::string("Incorrect version string: ") + min_version);
		}
	}

	std::unordered_map<std::string, Package> result;

	while (stmt.executeStep()) {
		const std::string ver_str = stmt.getColumn("version").getString();
		const int id = stmt.getColumn("id").getInt();

		if (has_min_ver) {
			semver::version v;
			try {
				v = semver::version::parse(ver_str);
			} catch (...) {
				LOG_WARN(std::string("Incorrect version in database table "
									 "packages with index :") +
						 std::to_string(id));
			}
			if (v < min_ver) {
				continue;
			}

			Package p;
			// clang-format off
			p.id            = id;
			p.name          = stmt.getColumn("name").getString();
			p.pkg_namespace = stmt.getColumn("namespace").getString();
			p.version       = ver_str;
			p.path          = stmt.getColumn("path").getString();
			p.src_type      = stmt.getColumn("source_type").getString();
			p.pkg_type      = stmt.getColumn("pkg_type").getString();
			p.created_at    = stmt.getColumn("created_at").getInt64();
			// clang-format on

			result[p.name] = p;
		}
	}

	return result;
}

void DataBase::upsert_package(Package &pkg) {
	// 1) Валидация входных данных
	if (pkg.name.empty() || pkg.version.empty() || pkg.pkg_namespace.empty() ||
		pkg.pkg_type.empty() || pkg.src_type.empty()) {
		throw DataBaseError(
			"Can't upload package to local repo, package data is incomplete",
			DataBaseErrorCode::INVAL_PKG);
	}

	// 2) Транзакция
	SQLite::Transaction txn(db);

	// 3) UPSERT в packages c RETURNING id
	SQLite::Statement upsertPkg(db, R"SQL(
        INSERT INTO packages
            (namespace, name, version, path, source_type, pkg_type, updated_at, deleted)
        VALUES
            (:ns, :name, :ver, :path, :src, :pkg, strftime('%s','now'), FALSE)
        ON CONFLICT(namespace, name, version) DO UPDATE SET
            path        = excluded.path,
            source_type = excluded.source_type,
            pkg_type    = excluded.pkg_type,
            updated_at  = strftime('%s','now'),
            deleted     = FALSE
        RETURNING id
    )SQL");

	upsertPkg.bind(":ns", pkg.pkg_namespace);
	upsertPkg.bind(":name", pkg.name);
	upsertPkg.bind(":ver", pkg.version);
	upsertPkg.bind(":path", pkg.path);
	upsertPkg.bind(":src", pkg.src_type);
	upsertPkg.bind(":pkg", pkg.pkg_type);

	int64_t packageId = -1;
	if (upsertPkg.executeStep()) {
		packageId = upsertPkg.getColumn(0).getInt64();
		// На всякий случай дочитываем все возможные строки из RETURNING
		while (upsertPkg.executeStep()) { /* consume */
		}
	} else {
		upsertPkg.reset();
		upsertPkg.clearBindings();
		throw DataBaseError("Failed to upsert package (no id returned)",
							DataBaseErrorCode::QUERY_FAILURE);
	}
	// ВАЖНО: закрываем курсор до любых последующих операций/commit
	upsertPkg.reset();
	upsertPkg.clearBindings();

	// 4) Пересбор зависимостей: сначала очищаем, затем вставляем актуальные
	{
		SQLite::Statement delDeps(
			db, "DELETE FROM dependencies WHERE package_id = :pid");
		delDeps.bind(":pid", packageId);
		delDeps.exec(); // exec() не оставляет открытых курсоров
	}

	if (!pkg.deps.empty()) {
		SQLite::Statement insDep(db, R"SQL(
            INSERT INTO dependencies
                (package_id, dep_namespace, dep_name, "constraint", optional)
            VALUES
                (:pid, :dns, :dname, :cstr, :opt)
        )SQL");

		for (const auto &dependency : pkg.deps) {
			insDep.bind(":pid", packageId);
			insDep.bind(":dns", dependency.dep_namespace.empty()
									? std::string("default")
									: dependency.dep_namespace);
			insDep.bind(":dname", dependency.dep_name);
			if (dependency.ver_constraint.empty()) {
				insDep.bind(":cstr"); // NULL
			} else {
				insDep.bind(":cstr", dependency.ver_constraint);
			}
			insDep.bind(":opt", static_cast<int>(dependency.optional)); // 0/1

			insDep.exec();
			insDep.reset();
			insDep.clearBindings();
		}
	}

	// 5) Коммит
	txn.commit();
}

} // namespace localpm::database
