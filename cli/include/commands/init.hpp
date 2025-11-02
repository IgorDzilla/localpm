
#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>

namespace localpm::cli {

class InitCommand : public Command {
public:
  std::string name() const override { return "init"; }
  std::string description() const override {
    return "Инициализация репозитория локальных пакетов";
  }

  void configure(CLI::App &sub) override {
    sub.add_option("-d,--dir", dir_, "Каталог для инициализации")
        ->default_val(".");
    sub.add_flag("--force", force_, "Перезаписать существующие файлы");
  }

  int run() override {
    namespace fs = std::filesystem;
    fs::path root = dir_;
    // ... здесь создаём структуру каталогов, конфиг и т.п.
    std::cout << "Initialized LocalPM repo at " << fs::absolute(root) << "\n";
    return 0;
  }

private:
  std::string dir_ = ".";
  bool force_ = false;
};

} // namespace localpm::cli

inline const bool registered_init =
    localpm::cli::CommandRegistry::instance().register_type<localpm::cli::InitCommand>();
