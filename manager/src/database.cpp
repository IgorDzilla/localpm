#include "database.hpp"
#include <fstream>
#include <sstream>

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

std::vector<Package> DataBase::search_package(std::string name_,
											  std::string version) {
	std::string query_string{"SELECT * FROM packages"};
	if (!name.empty()) {
		query_string +=
			" WHERE name = \'" + name + "\' AND version = \'" + version + "\';";
	}
	SQLite::Statement query(this->db, );
}
} // namespace localpm::manager::database
