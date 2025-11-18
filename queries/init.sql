PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS packages (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    name          TEXT    NOT NULL,  -- "logger"
    namespace     TEXT    NOT NULL,  -- "core"
    version       TEXT    NOT NULL,  -- нормализованный SemVer
    path          TEXT    NOT NULL,  -- абсолютный путь к каталогу версии
    source_type   TEXT    NOT NULL CHECK(source_type IN ('local','git','vendor','remote')),
    pkg_type      TEXT    NOT NULL CHECK(pkg_type IN ('static-lib','shared-lib','abi','header-only','other')),
    -- manifest_hash TEXT,              -- SHA256(manifest.toml)
    created_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
    updated_at    INTEGER NOT NULL DEFAULT (strftime('%s','now')),
    deleted       BOOL             DEFAULT (FALSE),
  UNIQUE(namespace, name, version)
);

CREATE TABLE IF NOT EXISTS dependencies (
  package_id    INTEGER NOT NULL,
  dep_namespace TEXT    NOT NULL,  -- если не указан, берётся "default"
  dep_name      TEXT    NOT NULL,
  ver_constraint    TEXT,              -- строка спецификации версий
  optional      INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY(package_id) REFERENCES packages(id) ON DELETE CASCADE
);

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
