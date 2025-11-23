#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <cstdint>
#include <filesystem>
#include <semver/semver.hpp>
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

namespace localpm::database {

enum class DataBaseErrorCode {
	QUERY_FILE_NOT_FOUND,
	QUERY_FAILURE,
	INVAL_PKG,
	PKG_EXISTS,
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

struct Dependency {
	std::string dep_namespace; // "default" если пусто
	std::string dep_name;
	std::string ver_constraint; // может быть пустой
	bool optional = false;

	Dependency() = default;
};

struct Package {
	size_t id;
	std::string name;
	std::string version;
	std::string pkg_namespace;
	std::string path;
	std::string src_type;
	std::string pkg_type;
	std::int64_t created_at;
	std::int64_t updated_at;
	bool deleted;

	// not in in table, but important
	std::vector<Dependency> deps;

	Package() = default;
};

class DataBase {
  private:
	SQLite::Database db;
	std::string path;

  public:
	DataBase(std::string &path);

	void init_db();
	auto search_packages(std::vector<std::string> namespaces = {},
						 std::vector<std::string> names = {},
						 std::string min_version = {})

		-> std::unordered_map<std::string, Package>;
	auto search_package_versions(std::string ns, std::string name,
								 std::string version) -> std::vector<Package>;

	void upsert_package(Package &pkg);
};
} // namespace localpm::database
