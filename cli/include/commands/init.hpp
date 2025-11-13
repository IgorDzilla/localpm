#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
//--- include lockfile --
#include "logger/logger.h"
#include <lockfile/lockfile.hpp>

namespace localpm::cli {

class InitCommand : public Command {
  public:
	std::string name() const override { return "init"; }
	std::string description() const override {
		return "initialize project at given directory (. by default)";
	}

	void configure(CLI::App &sub) override {
		sub.add_option("dir", dir_, "init dir")->default_val(".");
		sub.add_option("--schema", schema_, "lockfile schemal")->default_val(0);
		sub.add_flag("--force", force_, "overwrite project files");
	}

	int run() override {
		bool lockfile_exists = false;
		std::string filepath = create_filepath(dir_);

		if (FILE *fp = fopen(filepath.c_str(), "r")) {
			lockfile_exists = true;
		}

		if (lockfile_exists && !force_) {
			LOG_INFO("Lockfile already exists.");
			return 0;
		}

		try {
			if (std::filesystem::create_directories(dir_)) {
				LOG_INFO("Created project root dir.");
			}

			localpm::filesys::LockfileProcessor processor(filepath, schema_);
			processor.write_template();

		} catch (const std::filesystem::filesystem_error &err) {
			throw err;
		}

		LOG_INFO("Initialized new project at " + dir_ +
				 "with lockfile schema " + std::to_string(schema_));
		return 0;
	}

  private:
	std::string dir_ = ".";
	size_t schema_ = 0;
	bool force_ = false;

	std::string create_filepath(std::string &dir) {
		if (!dir.empty() && dir.back() == '/') {
			return dir + "lockfile.toml";
		}
		return dir + "/" + "lockfile.toml";
	}
};

} // namespace localpm::cli

inline const bool registered_init =
	localpm::cli::CommandRegistry::instance()
		.register_type<localpm::cli::InitCommand>();
