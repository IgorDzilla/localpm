#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace localpm::cli {

class AddCommand : public Command {
public:
  std::string name() const override { return "add"; }
  std::string description() const override {
    return "Добавить пакет в локальный реестр";
  }

  void configure(CLI::App &sub) override {
    sub.add_option("name", name_, "Name of package")->required();
    sub.add_option("-v,--version", version_, "version")
        ->default_val("latest");
    sub.add_option("--source", source_, "Source (local|vendor|git)")
        ->default_val("local");
    sub.add_option("--path", path_, "Path to source=local");
    sub.add_option("--url", url_, "URL for source=git / vendor");
    sub.add_flag("--replace", replace_, "Replace package");
  }

  int run() override {
    namespace fs = std::filesystem;
    fs::path db = fs::current_path() / "packages.txt";

    // open file
    std::ofstream out(db, std::ios::app);
    if (!out) {
        std::cerr << "[error] cannot open " << db << " for writing\n";
        return 1;
    }

    // пишем данные в файл
    out << name_ << " " << version_ << " " << source_
        << " " << (path_.empty() ? "-" : path_)
        << " " << (url_.empty() ? "-" : url_)
        << " " << (replace_ ? "replace" : "keep")
        << "\n";
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

inline const bool registered_add =
    localpm::cli::CommandRegistry::instance().register_type<localpm::cli::AddCommand>();
