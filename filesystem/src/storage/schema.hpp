#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

namespace localpm::storage {

// clang-format off
/*
	Structure of table Packages:
	CREATE TABLE IF NOT EXISTS packages (
		id            INTEGER PRIMARY KEY AUTOINCREMENT,
		name          TEXT    NOT NULL,  -- "logger"
  		namespace     TEXT    NOT NULL,  -- "core"
  		version       TEXT    NOT NULL,  -- normalized SemVer
  		path          TEXT    NOT NULL,  -- abs path to version directory
  		source_type   TEXT    NOT NULL CHECK(source_type IN ('local','git','vendor','remote')),
  		pkg_type      TEXT    NOT NULL CHECK(pkg_type IN ('static-lib','shared-lib','abi','header-only','other')),
  		manifest_hash TEXT,              -- SHA256(manifest.toml)
  		created_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
  		updated_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
  		is_deleted INTEGER DEFAULT 0,
  		UNIQUE(namespace, name, version)
	);	
	
*/
// clang-format on

struct Package {
	int id;
	std::string name;
	std::string namespace_;
	std::string version;
	std::string path;
	std::string source_type;
	std::string pkg_type;
	std::optional<std::string> manifest_hash;
	std::int64_t created_at;
	std::int64_t updated_at;
};

void create_packages_table(SQLite::Database db);

/*
	CREATE TABLE IF NOT EXISTS dependencies (
		package_id    INTEGER NOT NULL,
		dep_namespace TEXT    NOT NULL,  -- if not stated - "default"
		dep_name      TEXT    NOT NULL,
		constraint    TEXT,              -- specs for new version
		optional      INTEGER NOT NULL DEFAULT 0,
		FOREIGN KEY(package_id) REFERENCES packages(id) ON DELETE CASCADE
	);
*/

struct Dependencies {
	int package_id;
	std::string dep_namespace;
	std::string dep_name;
	std::string constraint;
	bool optional = false;
	int foreign_key;
};

void create_dependencies_table(SQLite::Database db);

/*
	CREATE INDEX IF NOT EXISTS idx_pkg_lookup
		ON packages(namespace, name, version);
	CREATE INDEX IF NOT EXISTS idx_pkg_name
		ON packages(namespace, name);
	CREATE INDEX IF NOT EXISTS idx_pkg_source
		ON packages(source_type);
	CREATE INDEX IF NOT EXISTS idx_deps_pkg
		ON dependencies(package_id);
	CREATE INDEX IF NOT EXISTS idx_deps_target
		ON dependencies(dep_namespace, dep_name);
 */
void create_indexes(SQLite::Database);

/*
	PRAGMA foreign_keys = ON;
 */
void enable_foreign_keys(SQLite::Database);
} // namespace localpm::storage
