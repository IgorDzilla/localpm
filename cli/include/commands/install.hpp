#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace localpm::cli {

class InstallCommand : public Command {
public: 
	std::string name() const override {
		return "install";
	}
	std::string description() const override { 
		return "Installing packages, added to local registry";
	}

	void configure(CLI::App& sub) override { 
		sub.add_flag("--force", force_, "Forced reinstallatoin of packages");
	}

	int run() override {
		namespace fs = std::filesystem;
		fs::path db = "./packages.txt";

		if (!fs::exists(db)) {
			std::cerr << "List for install is empty, please added packages using 'add'\n";
			return 1;
		}

		std::ifstream in(db);
		std::string name, version, source, path;
		while (in >> name >> version >> source >> path) {
			std::cout << "Installing " << name << "( "
					  << version << " ) from" << source
					  << (force_ ? " [force]" : "") << "\n";
		}

		std::cout << "Install succesful!\n";
		return 0;
	}

private:
	bool force_ = false;
};

} //namespace localpm::cli

inline const bool registered_install = 
	localpm::cli::CommandRegistry::instance().register_type<localpm::cli::InstallCommand>();
