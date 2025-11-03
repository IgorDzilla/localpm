#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace localpm::cli {

class ListCommand : public Command {
public: 
	std::string name() const override{
		return "list";
	}
	std::string description() const override{
    	return "Show all packages added to the local registry";
    } 
    void configure(CLI::App &) override {}

	int run() override {
		namespace fs = std::filesystem;
		fs::path db = "./packages.txt";

		if (!fs::exists(db)) {
			std::cout << "List is empty, please add at least one package using 'add'. \n";
			return 0;
		}

		std::ifstream in(db);
		std::string name, version, source, path;
		std::cout << "=== PACKAGE LIST === \n";
		
		while (in >> name >> version >> source >> path) {
		  std::cout << " - " << name << " (" << version << "), source: " << source
		            << ", path: " << path << "\n";
		}

		std::cout << "==================== \n";
		return 0;
	}
};

}

inline const bool registered_list =
    localpm::cli::CommandRegistry::instance().register_type<localpm::cli::ListCommand>();
