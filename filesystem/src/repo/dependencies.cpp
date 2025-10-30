// src/repo/dependencies.cpp
#include "dependencies.hpp"

namespace localpm::repo {

void Dependencies::add(int package_id, const std::string &dep_ns,
                       const std::string &dep_name,
                       const std::optional<std::string> &constraint,
                       bool optional) {
  st_.execute("INSERT INTO dependencies(package_id, dep_namespace, dep_name, "
              "constraint, optional) "
              "VALUES(?,?,?,?,?);",
              package_id, dep_ns, dep_name, constraint ? *constraint : nullptr,
              optional ? 1 : 0);
}

} // namespace localpm::repo
