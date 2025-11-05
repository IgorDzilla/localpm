#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
//--- include lockfile --
#include "lockfile.hpp"
#include "logger/logger.hpp"

namespace localpm::cli {

class InitCommand : public Command {
  public:
	std::string name() const override { return "init"; }
	std::string description() const override {
		return "initialize project at given directory (. by default)";
	}

	void configure(CLI::App &sub) override {

		sub.add_option("dir", dir_, "init dir")->default_val(".");
		sub.add_flag("--force", force_, "overwrite project files");
	}

	int run() override {
		bool lockfile_exists = false;
		if (FILE *fp = fopen("lockfile.toml", "r")) {
			lockfile_exists = true;
		}

		if (lockfile_exists && !force_) {
			return 0;
		}

		namespace fs = std::filesystem;

		fs::path root = dir_;
		std::cout << "Initialized LocalPM project at " << fs::absolute(root)
				  << "\n";
		return 0;
	}

  private:
	std::string dir_ = ".";
	bool force_ = false;
};

} // namespace localpm::cli

inline const bool registered_init =
	localpm::cli::CommandRegistry::instance()
		.register_type<localpm::cli::InitCommand>();
