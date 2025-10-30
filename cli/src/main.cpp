#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "commands_all.hpp"
#include "registry.hpp"

using namespace localpm::cli;

int main(int argc, char **argv) {
  CLI::App app{"LocalPM — Local Package Manager for C/C++"};
  app.require_subcommand(1); // min one subcommand

  // Глобальные флаги
  std::string config_path;
  bool verbose = false;
  app.add_option("-c, --config", config_path, "Path to config file");
  app.add_flag("-v, --verbose", verbose, "Подробный вывод");

  // create commands and get it subcommands
  auto commands = CommandRegistry::instance().instantiate_all();

  std::unordered_map<CLI::App *, Command *> sub_to_cmd;

  for (auto &cmd_ptr : commands) {
    auto *raw = cmd_ptr.get();
    CLI::App *sub = app.add_subcommand(raw->name(), raw->description());

    // if cmd need personal options compile it in subcommand
    raw->configure(*sub);
    sub_to_cmd.emplace(sub,
                       raw); // push elem if key new and unique, else - error
  }

  try {
    app.parse(argc, argv);

    // find which subcommand requests
    for (auto &[sub, cmd] : sub_to_cmd) {
      if (sub->parsed()) {
        // there may save or give global flags in command with using set()
        return cmd->run();
      }
    }

    // command not fount if we can go there
    std::cout << app.help() << "\n";
    return 1;

  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  } catch (const std::exception &e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 2;
  }
}
