// src/storage/migrations.hpp
#pragma once
#include "db.hpp"

namespace localpm::storage {
int current_version(Storage &st); // читаем meta.schema_version
void run_migrations(Storage &st); // применяем нужные ALTERы
} // namespace localpm::storage
