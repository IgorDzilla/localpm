// src/repo/packages.hpp
#pragma once
#include "../storage/db.hpp"
#include <optional>
#include <string>

namespace localpm::repo {

class Packages {
public:
  explicit Packages(localpm::storage::Storage &st) : st_(st) {}
  int upsert(const std::string &name, const std::string &ns,
             const std::string &version, const std::string &path,
             const std::string &source_type, const std::string &pkg_type,
             const std::optional<std::string> &manifest_hash);

private:
  localpm::storage::Storage &st_;
};

} // namespace localpm::repo
