// src/storage/schema.cpp
#include "schema.hpp"

namespace localpm::storage {

void create_base_schema(Storage &st) {
  st.execute(R"SQL(
CREATE TABLE IF NOT EXISTS packages (
  id            INTEGER PRIMARY KEY AUTOINCREMENT,
  name          TEXT    NOT NULL,
  namespace     TEXT    NOT NULL,
  version       TEXT    NOT NULL,
  path          TEXT    NOT NULL,
  source_type   TEXT    NOT NULL CHECK(source_type IN ('local','git','vendor','remote')),
  pkg_type      TEXT    NOT NULL CHECK(pkg_type IN ('static-lib','shared-lib','abi','header-only','other')),
  manifest_hash TEXT,
  created_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
  updated_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
  UNIQUE(namespace, name, version)
);
)SQL");

  st.execute(R"SQL(
CREATE TABLE IF NOT EXISTS dependencies (
  package_id    INTEGER NOT NULL,
  dep_namespace TEXT    NOT NULL,
  dep_name      TEXT    NOT NULL,
  constraint    TEXT,
  optional      INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY(package_id) REFERENCES packages(id) ON DELETE CASCADE
);
)SQL");
}

void create_indexes(Storage &st) {
  st.execute(
      R"(CREATE INDEX IF NOT EXISTS idx_pkg_lookup  ON packages(namespace, name, version);)");
  st.execute(
      R"(CREATE INDEX IF NOT EXISTS idx_pkg_name    ON packages(namespace, name);)");
  st.execute(
      R"(CREATE INDEX IF NOT EXISTS idx_pkg_source  ON packages(source_type);)");
  st.execute(
      R"(CREATE INDEX IF NOT EXISTS idx_deps_pkg    ON dependencies(package_id);)");
  st.execute(
      R"(CREATE INDEX IF NOT EXISTS idx_deps_target ON dependencies(dep_namespace, dep_name);)");
}

// авто-обновление updated_at на UPDATE
void create_triggers(Storage &st) {
  st.execute(R"SQL(
CREATE TRIGGER IF NOT EXISTS packages_set_updated_at
AFTER UPDATE ON packages
FOR EACH ROW
BEGIN
  UPDATE packages SET updated_at = strftime('%s','now') WHERE id = NEW.id;
END;
)SQL");
}

void ensure_meta_table(Storage &st) {
  st.execute(
      R"(CREATE TABLE IF NOT EXISTS meta (key TEXT PRIMARY KEY, value TEXT NOT NULL);)");
  st.execute(
      R"(INSERT OR IGNORE INTO meta(key,value) VALUES('schema_version','1');)");
}

} // namespace localpm::storage
