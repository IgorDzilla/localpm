// src/repo/packages.cpp
#include "packages.hpp"

namespace localpm::repo {

int Packages::upsert(const std::string &name, const std::string &ns,
                     const std::string &version, const std::string &path,
                     const std::string &source_type,
                     const std::string &pkg_type,
                     const std::optional<std::string> &manifest_hash) {
  // UPSERT по (namespace, name, version)
  // sqlite_orm не даёт DSL для UPSERT → используем raw SQL
  std::string sql =
      "INSERT INTO "
      "packages(namespace,name,version,path,source_type,pkg_type,manifest_hash)"
      " "
      "VALUES(?,?,?,?,?,?,?) "
      "ON CONFLICT(namespace,name,version) DO UPDATE SET "
      "  path=excluded.path, source_type=excluded.source_type, "
      "  pkg_type=excluded.pkg_type, manifest_hash=excluded.manifest_hash;";

  st_.begin_transaction();
  try {
    st_.execute(sql, ns, name, version, path, source_type, pkg_type,
                manifest_hash ? *manifest_hash : nullptr);

    // Получить id (ROWID) текущей записи
    int id = 0;
    st_.select(
        "SELECT id FROM packages WHERE namespace=? AND name=? AND version=?;",
        [&](int rowid) { id = rowid; }, ns, name, version);
    st_.commit();
    return id;
  } catch (...) {
    st_.rollback();
    throw;
  }
}

} // namespace localpm::repo
