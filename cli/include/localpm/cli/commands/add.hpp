
#pragma once
#include "localpm/cli/registry.hpp"
#include <CLI/CLI.hpp>
#include <iostream>

namespace localpm::cli {

class AddCommand : public Command {
public:
  std::string name() const override { return "add"; }
  std::string description() const override {
    return "Добавить пакет в локальный реестр";
  }

  void configure(CLI::App &sub) override {
    sub.add_option("name", name_, "Имя пакета")->required();
    sub.add_option("-v,--version", version_, "Версия (семвер)")
        ->default_val("latest");
    sub.add_option("--source", source_, "Источник (local|vendor|git)")
        ->default_val("local");
    sub.add_option("--path", path_, "Путь для source=local");
    sub.add_option("--url", url_, "URL для source=git / vendor");
    sub.add_flag("--replace", replace_, "Заменить, если уже существует");
  }

  int run() override {
    // Здесь можно дернуть слой домена/индекса, который ты уже проектировал.
    std::cout << "Add: name=" << name_ << " version=" << version_
              << " source=" << source_
              << (path_.empty() ? "" : " path=" + path_)
              << (url_.empty() ? "" : " url=" + url_)
              << (replace_ ? " [replace]" : "") << "\n";
    return 0;
  }

private:
  std::string name_;
  std::string version_;
  std::string source_;
  std::string path_;
  std::string url_;
  bool replace_ = false;
};

} // namespace localpm::cli

REGISTER_COMMAND(localpm::cli::AddCommand);
