// src/storage/db.cpp
#include "db.hpp"
#include "migrations.hpp"
#include "schema.hpp"

namespace localpm::storage {

std::unique_ptr<Storage> open_or_create(const std::string &path) {
  auto st = std::make_unique<Storage>(sqlite_orm::make_storage(path));
  st->execute("PRAGMA foreign_keys = ON;");
  st->execute("PRAGMA journal_mode = WAL;");
  st->execute("PRAGMA synchronous = NORMAL;");
  return st;
}

void apply_schema_and_migrations(Storage &st) {
  // Базовая схема + индексы + триггеры
  create_base_schema(st);
  create_indexes(st);
  create_triggers(st);

  // Версионирование (по желанию)
  ensure_meta_table(st);
  run_migrations(st);
}

} // namespace localpm::storage
