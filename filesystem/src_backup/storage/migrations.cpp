// src/storage/migrations.cpp
#include "migrations.hpp"
#include <string>

namespace localpm::storage {

int current_version(Storage &st) {
  int v = 0;
  bool found = false;
  st.select("SELECT value FROM meta WHERE key='schema_version';",
            [&](const std::string &s) {
              v = std::stoi(s);
              found = true;
            });
  return found ? v : 0;
}

static void set_version(Storage &st, int v) {
  st.execute("INSERT INTO meta(key,value) VALUES('schema_version', '" +
             std::to_string(v) + "') ON CONFLICT(key) DO UPDATE SET value='" +
             std::to_string(v) + "';");
}

void run_migrations(Storage &st) {
  int v = current_version(st);
  // пример: будущие миграции
  switch (v) {
  case 1:
    // st.execute("ALTER TABLE packages ADD COLUMN ...;");
    set_version(st, 2);
    [[fallthrough]];
  default:
    break;
  }
}

} // namespace localpm::storage
