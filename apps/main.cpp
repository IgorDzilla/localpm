#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "commands_all.hpp"
#include "registry.hpp"

// Optionally include lockfile for testing
#include "lockfile.hpp"

using namespace localpm::cli;

int main(int argc, char **argv) {
	CLI::App app{"LocalPM — Local Package Manager for C/C++"};
	app.require_subcommand(1); // Require at least one subcommand

	// Global flags
	std::string config_path;
	bool verbose = false;
	app.add_option("-c,--config", config_path, "Path to config file");
	app.add_flag("-v,--verbose", verbose, "Verbose output");

	// Create commands and get their subcommands
	auto commands = CommandRegistry::instance().instantiate_all();

	std::unordered_map<CLI::App *, Command *> sub_to_cmd;

	for (auto &cmd_ptr : commands) {
		auto *raw = cmd_ptr.get();
		CLI::App *sub = app.add_subcommand(raw->name(), raw->description());

		// Configure command-specific options
		raw->configure(*sub);
		sub_to_cmd.emplace(sub, raw);
	}

	try {
		app.parse(argc, argv);

		// Find which subcommand was requested
		for (auto &[sub, cmd] : sub_to_cmd) {
			if (sub->parsed()) {
				// Global flags could be passed to commands here if needed
				if (verbose) {
					std::cout << "[verbose] Running command: " << cmd->name()
							  << "\n";
				}
				return cmd->run();
			}
		}

		// If we reach here, no command was parsed (shouldn't happen due to
		// require_subcommand)
		std::cout << app.help() << "\n";
		return 1;

	} catch (const CLI::ParseError &e) {
		return app.exit(e);
	} catch (const LockfileError &e) {
		std::cerr << "Lockfile error: " << e.what() << "\n";
		return 2;
	} catch (const std::exception &e) {
		std::cerr << "Fatal error: " << e.what() << "\n";
		return 3;
	}
}
