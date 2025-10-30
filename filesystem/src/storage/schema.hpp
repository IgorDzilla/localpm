// src/storage/schema.hpp
#pragma once
#include "db.hpp"

namespace localpm::storage {
void create_base_schema(Storage &st);
void create_indexes(Storage &st);
void create_triggers(Storage &st);
void ensure_meta_table(Storage &st);
} // namespace localpm::storage
