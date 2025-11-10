#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef DB_PATH
#define DB_PATH "~/.local/localpm/index.db3"
#endif // !DB_ROOT

#ifndef DB_QUERY_FOLDER
#define DB_QUERY_FOLDER "~/.local/localpm/queries"
#endif // !DB_QUERY_FOLDER

#ifndef INIT_QUERY
#define INIT_QUERY "init.sql"
#endif // !INIT_QUERY

namespace localpm::manager::database {

enum class DataBaseErrorCode {
	QUERY_FILE_NOT_FOUND,
	QUERY_FAILURE,
};

class DataBaseError : public std::exception {
  private:
	std::string msg;
	DataBaseErrorCode code;

  public:
	DataBaseError(std::string msg, DataBaseErrorCode code)
		: msg(msg), code(code) {}
	DataBaseError(const char *msg_, const char *exception_msg,
				  DataBaseErrorCode code)
		: code(code) {
		msg = std::string(msg_) + ":\t" + std::string(exception_msg);
	}
	const char *what() const noexcept override { return msg.c_str(); }
};

struct Package {
	size_t id;
	std::string name = {};
	std::string version = {};
	std::string pkg_namespace = {};
	std::string path = {};
	std::string src_type = {};
	std::string pkg_type = {};
	std::string created_at = {};
	std::string updated_at = {};
	bool deleted;
};

class DataBase {
  private:
	SQLite::Database db;
	std::string path;
	void init_db();

  public:
	DataBase(std::string &path);
	auto search_package(std::string name = {}, std::string version = {})
		-> std::unordered_map<std::string, Package>;
};

} // namespace localpm::manager::database
