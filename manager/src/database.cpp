#include "database.hpp"
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
} // namespace localpm::manager::database
