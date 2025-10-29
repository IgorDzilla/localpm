#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  CLI::App app{"Tiny demo with CLI11"};

  // Опции
  std::string name = "world";
  app.add_option("-n,--name", name, "Name to greet");

  bool shout = false;
  app.add_flag("-s,--shout", shout, "Uppercase the greeting");

  // Парсинг + авто --help/--version
  CLI11_PARSE(app, argc, argv);

  std::string msg = "Hello, " + name + "!";
  if (shout) {
    for (auto &ch : msg)
      ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  }
  std::cout << msg << std::endl;
  return 0;
}
