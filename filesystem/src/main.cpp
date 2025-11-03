#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>

int main() {
  try {
    // Открываем (или создаём) файл базы данных
    SQLite::Database db("test.db",
                        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    // Создаём таблицу, если нет
    db.exec(
        "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value TEXT)");

    // Вставляем строку
    db.exec("INSERT INTO test (value) VALUES ('Hello, SQLiteCpp!')");

    // Читаем первую строку
    SQLite::Statement query(db, "SELECT id, value FROM test");
    while (query.executeStep()) {
      std::cout << query.getColumn(0).getInt() << " | "
                << query.getColumn(1).getText() << '\n';
    }
  } catch (const std::exception &e) {
    std::cerr << "DB error: " << e.what() << '\n';
    return 1;
  }

  return 0;
}
