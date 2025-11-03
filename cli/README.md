# CLI.md

This file provides guidance to WARP (warp.dev) and contributors when working with the **CLI parser** component of this repository.

---

## Table of Contents

* [Project Overview](#project-overview)
* [Development Commands](#development-commands)

  * [Building](#building)
  * [Code Formatting](#code-formatting)
  * [Testing](#testing)
* [Architecture](#architecture)

  * [Core Components](#core-components)
  * [Current Commands](#current-commands)
* [Creating Your Own Command](#creating-your-own-command)

  * [Command Structure](#command-structure)
  * [Example Template](#example-template)
  * [Registration and Inclusion](#registration-and-inclusion)
* [Design Notes](#design-notes)
* [File Structure](#file-structure)
* [Dependencies](#dependencies)
* [Example Session](#example-session)
* [Project-Specific Notes](#project-specific-notes)

---

## Project Overview

`localpm` CLI is a lightweight command-line interface for the Local Package Manager prototype.
It is built on top of **[CLI11](https://github.com/CLIUtils/CLI11)** and implements a modular command registration system that allows new commands to be added without modifying the main entry point.

This component serves as the front-end for all developer-facing actions such as initializing repositories, adding packages, and future features like building and publishing.

---

## Development Commands

### Building

#### Using CMake

```bash
mkdir -p build
cd build
cmake ..
make
./localpm --help
```

#### Manual Compilation

```bash
clang++ -std=c++17 -Wall -Wextra -Wpedantic \
  -Iinclude -Iexternal/CLI11/include \
  src/main.cpp -o localpm
```

---

### Code Formatting

```bash
clang-format -i src/*.cpp include/**/*.hpp
```

---

### Testing

```bash
./localpm init --dir .
./localpm add mylib -v 1.2.3 --source local --path ./libs/mylib
```

---

## Architecture

### Core Components

**Command Registry** (`registry.hpp`)

* Manages dynamic command registration via static initialization.
* Allows adding new commands with `REGISTER_COMMAND(MyCommand);`.

**Commands** (`commands/*.hpp`)

* Each command implements the base interface:

  * `std::string name() const` — command identifier (`"init"`, `"add"`)
  * `std::string description() const` — short help text
  * `void configure(CLI::App&)` — define options and flags
  * `int run()` — perform execution and return exit code

**Main Application** (`src/main.cpp`)

* Defines global flags (`--config`, `--verbose`)
* Registers subcommands and dispatches to the selected one.

---

### Current Commands

| Command  | Description                           | Example                                                         |
| -------- | ------------------------------------- | --------------------------------------------------------------- |
| **init** | Initialize a new localpm repository   | `localpm init --dir . --force`                                  |
| **add**  | Add a package to the local repository | `localpm add mylib -v 1.0.0 --source local --path ./libs/mylib` |

Planned future commands:

* `remove` — remove a package
* `build` — build registered packages
* `publish` — upload packages

---

## Creating Your Own Command

You can easily extend `localpm` by creating a new header file in
`include/localpm/cli/commands/` (for example, `remove.hpp`).

### Command Structure

Each command must **inherit** from `localpm::cli::Command` and implement **four** virtual methods:

| Method                            | Required                      | Description                                                       |
| --------------------------------- | ----------------------------- | ----------------------------------------------------------------- |
| `std::string name() const`        | ✅ **Required**                | Returns the subcommand name (`"remove"`, `"build"`, etc.)         |
| `std::string description() const` | ✅ **Required**                | One-line help text for `--help`                                   |
| `void configure(CLI::App &sub)`   | ⚙️ *Optional but recommended* | Adds flags and options to the command using CLI11                 |
| `int run()`                       | ✅ **Required**                | Executes the command logic and returns an exit code (0 = success) |

If your command needs no options, you can leave `configure()` empty.

---

### Example Template

Here’s a minimal command implementation you can copy to start a new command:

```cpp
#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <iostream>

namespace localpm::cli {

class RemoveCommand : public Command {
public:
  std::string name() const override { return "remove"; }

  std::string description() const override {
    return "Remove a package from the local repository";
  }

  void configure(CLI::App &sub) override {
    sub.add_option("name", name_, "Name of the package to remove")->required();
    sub.add_flag("--force", force_, "Force remove even if used by others");
  }

  int run() override {
    std::cout << "Removing package: " << name_
              << (force_ ? " (forced)" : "") << std::endl;
    // TODO: connect with package registry
    return 0;
  }

private:
  std::string name_;
  bool force_ = false;
};

} // namespace localpm::cli

// Static registration — required for the command to appear in CLI
REGISTER_COMMAND(localpm::cli::RemoveCommand);
```

---

### Registration and Inclusion

After creating your `*.hpp` file, include it in
`include/commands_all.hpp`:

```cpp
#pragma once
#include "commands/add.hpp"
#include "commands/init.hpp"
#include "commands/remove.hpp"  // <-- new command
```

That’s all — no edits to `main.cpp` are needed.
Your command will automatically appear in `--help` output and behave as a proper subcommand.

---

## Design Notes

* **Header-only registration:** handled by static `REGISTER_COMMAND`.
* **Zero-modification principle:** no edits to `main.cpp` required.
* **Thread-safe registry:** protected by `std::mutex`.
* **C++20 features:** structured bindings, `std::filesystem`, lambdas, etc.
* **Future context injection:** for passing global config to commands.

---

## File Structure

```
CLIParse/
├─ CMakeLists.txt
├─ src/main.cpp
├─ include/
│  ├─ registry.hpp
│  ├─ commands/
│  │  ├─ add.hpp
│  │  ├─ init.hpp
│  │  ├─ remove.hpp      # <- new custom command
│  │  └─ ...
│  └─ commands_all.hpp
└─ external/CLI11/
```

---

## Dependencies

* **CLI11** — header-only CLI parser
* **C++17 STL** — `filesystem`, `unordered_map`, `unique_ptr`, etc.
* No runtime dependencies.

---

## Example Session

```bash
$ ./localpm --help
LocalPM — Local Package Manager for C/C++
Usage: localpm [OPTIONS] SUBCOMMAND

Options:
  -c,--config TEXT    Path to config file
  -v,--verbose        Verbose output

Subcommands:
  init      Initialize repository
  add       Add package to local registry
  remove    Remove a package from repository
```

---

## Project-Specific Notes

* The CLI parser is currently in **prototype stage**.
* Integration with the lockfile and backend logic is planned.
* Commands are loaded at compile time, requiring no runtime discovery.
* Future improvements include configuration context and logging injection.

