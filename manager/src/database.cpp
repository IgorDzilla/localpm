#include "database.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace localpm::manager::database {

// --- util ---
static std::string assemble_path(std::string path, std::string filename) {
	if (!path.empty() && path.back() != '/') {
		return path + "/" + filename;
	}

	return path + filename;
}

DataBase::DataBase(std::string &path_par) : path(path_par), db(path_par) {}

void DataBase::init_db() {
	std::string query_path =
		assemble_path(std::string(DB_QUERY_FOLDER), std::string(INIT_QUERY));
	std::fstream input(query_path);

	if (!input) {
		throw DataBaseError("Initialization query file not found",
							DataBaseErrorCode::QUERY_FILE_NOT_FOUND);
	}

	std::stringstream buffer;
	buffer << input.rdbuf();
	try {
		db.exec(buffer.str());
	} catch (std::exception &e) {
		std::string err_str =
			std::string("Query execution failed: ") + std::string(e.what());
		throw DataBaseError(err_str, DataBaseErrorCode::QUERY_FAILURE);
	}
}

/*
 * Executes select query.
 * TODO: search should be better, now only equality is checked.
 */
std::unordered_map<std::string, Package>
DataBase::search_package(std::string name, std::string version) {
	std::string query_string{"SELECT * FROM packages"};
	if (!name.empty() && !version.empty()) {
		query_string +=
			"WHERE name = \'" + name + "\' AND  version = \'" + version + "\'";
	} else if (!name.empty()) {
		query_string += "WHERE name = \'" + name + "\'";
	} else if (!version.empty()) {

		query_string += "WHERE version = \'" + version + "\'";
	}

	query_string += ";";

	std::cout << "Query string:\t" << query_string << std::endl;

	SQLite::Statement query(db, query_string);
	std::unordered_map<std::string, Package> pkg_map;

	try {
		while (query.executeStep()) {
			Package pkg;

			pkg.id = static_cast<int>(query.getColumn(0));
			pkg.name = static_cast<const char *>(query.getColumn(1));
			pkg.version = static_cast<const char *>(query.getColumn(1));
			pkg.pkg_namespace = static_cast<const char *>(query.getColumn(1));
			pkg.path = static_cast<const char *>(query.getColumn(1));
			pkg.src_type = static_cast<const char *>(query.getColumn(1));
			pkg.pkg_type = static_cast<const char *>(query.getColumn(1));
			pkg.created_at = static_cast<const char *>(query.getColumn(1));
			pkg.updated_at = static_cast<const char *>(query.getColumn(1));
			pkg.deleted = static_cast<int>(query.getColumn(
				1)); // who the fuck knows how to convert this shit

			pkg_map[pkg.name] = pkg;
		}
	} catch (std::exception &e) {
		throw DataBaseError("Query execution failed", e.what(),
							DataBaseErrorCode::QUERY_FAILURE);
	}

	return pkg_map;
}

void DataBase::insert_package(Package &pkg, std::filesystem::path &path) {
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
	//    deleted=FALSE — «разархивируем» запись, если была помечена удалённой
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
	upsertPkg.bind(":path", path.u8string()); // безопасно для non-ASCII
	upsertPkg.bind(":src", pkg.src_type);
	upsertPkg.bind(":pkg", pkg.pkg_type);

	int64_t packageId = -1;
	if (upsertPkg.executeStep()) {
		packageId = upsertPkg.getColumn(0).getInt64();
	} else {
		throw DataBaseError("Failed to upsert package (no id returned)",
							DataBaseErrorCode::DB_ERROR);
	}

	// 4) Пересобираем зависимости: сначала удаляем, потом вставляем
	{
		SQLite::Statement delDeps(
			db, "DELETE FROM dependencies WHERE package_id = :pid");
		delDeps.bind(":pid", packageId);
		delDeps.exec();
	}

	if (!pkg.deps.empty()) {
		SQLite::Statement insDep(db, R"SQL(
            INSERT INTO dependencies
                (package_id, dep_namespace, dep_name, constraint, optional)
            VALUES
                (:pid, :dns, :dname, :cstr, :opt)
        )SQL");

		for (const auto &d : pkg.deps) {
			insDep.bind(":pid", packageId);
			insDep.bind(":dns", d.dep_namespace.empty() ? std::string("default")
														: d.dep_namespace);
			insDep.bind(":dname", d.dep_name);
			// Поле называется "constraint" — допустимо в SQLite, но это
			// зарезервированное слово в общем SQL. Если когда-то переедете на
			// другую СУБД, лучше переименовать, напр. "ver_constraint".
			if (d.ver_constraint.empty())
				insDep.bind(":cstr"); // bind NULL
			else
				insDep.bind(":cstr", d.ver_constraint);

			insDep.bind(":opt", static_cast<int>(d.optional));
			insDep.exec();
			insDep.reset();
			insDep.clearBindings();
		}
	}

	// 5) Коммит
	txn.commit();
}
} // namespace localpm::manager::database
