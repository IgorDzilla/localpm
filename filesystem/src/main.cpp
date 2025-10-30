#include <iostream>
#include <sqlite_orm/sqlite_orm.h>
#include <string>

struct Package {
  int id;
  std::string name;
  std::string version;
  std::string kind;
};

int main() {
  std::cout << "Hello world\n";
  try {
    auto storage = sqlite_orm::make_storage(
        "test.db",
        sqlite_orm::make_table(
            "package",
            sqlite_orm::make_column("id", &Package::id,
                                    sqlite_orm::primary_key().autoincrement()),
            sqlite_orm::make_column("name", &Package::name),
            sqlite_orm::make_column("version", &Package::version),
            sqlite_orm::make_column("kind", &Package::kind),
            sqlite_orm::unique(
                &Package::name,
                &Package::version) // <-- важно: sqlite_orm::unique
            ));

    storage.sync_schema();

    Package p{0, "fmt", "10.2.1", "header-only"};
    p.id = storage.insert(p);

    for (const auto &row : storage.get_all<Package>(
             sqlite_orm::where(sqlite_orm::c(&Package::name) == "fmt"))) {
      std::cout << row.id << " | " << row.name << " " << row.version << " | "
                << row.kind << "\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "DB error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
