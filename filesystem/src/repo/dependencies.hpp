// src/repo/dependencies.hpp
#pragma once
#include "../storage/db.hpp"
#include <optional>
#include <string>

namespace localpm::repo {

class Dependencies {
public:
  explicit Dependencies(localpm::storage::Storage &st) : st_(st) {}
  void add(int package_id, const std::string &dep_ns,
           const std::string &dep_name,
           const std::optional<std::string> &constraint, bool optional);

private:
  localpm::storage::Storage &st_;
};

} // namespace localpm::repo
