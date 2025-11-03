#pragma once
#include <memory>
#include <sqlite_orm/sqlite_orm.h>

namespace localpm::storage {

using Storage = decltype(sqlite_orm::make_storage(""));

std::unique_ptr<Storage> open_or_create(const std::string &path);
void apply_schema_and_migrations(Storage &st);

} // namespace localpm::storage
