
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
# Generate build files and compile
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Run the CLI
./build/localpm --help
```

#### Manual Compilation (for prototyping)

```bash
clang++ -std=c++20 -Wall -Wextra -Wpedantic \
  -Iinclude -Iexternal/CLI11/include \
  src/main.cpp -o localpm
```

---

### Code Formatting

Use `clang-format` for all CLI sources:

```bash
clang-format -i src/*.cpp include/localpm/cli/**/*.hpp
```

---

### Testing

Currently there are no automated tests.
Commands can be tested manually:

```bash
./localpm init --dir .
./localpm add mylib -v 1.2.3 --source local --path ./libs/mylib
```

---

## Architecture

### Core Components

**Command Registry** (`registry.hpp`)

* Manages dynamic command registration via static initialization.
* Allows adding new commands with a single line:
  `REGISTER_COMMAND(MyCommand);`
* Stores factory lambdas to instantiate all registered command classes.

**Commands** (`commands/*.hpp`)

* Each command implements the base interface:

  * `std::string name() const` — command identifier (`"init"`, `"add"`)
  * `std::string description() const` — help text for CLI
  * `void configure(CLI::App&)` — define options and flags
  * `int run()` — execute command logic and return exit code

**Main Application** (`src/main.cpp`)

* Defines global flags (`--config`, `--verbose`)
* Creates CLI11 `App`, registers subcommands, and dispatches to the selected one.
* Acts as the single entry point of the CLI binary.

---

### Current Commands

| Command  | Description                           | Example                                                         |
| -------- | ------------------------------------- | --------------------------------------------------------------- |
| **init** | Initialize a new localpm repository   | `localpm init --dir . --force`                                  |
| **add**  | Add a package to the local repository | `localpm add mylib -v 1.0.0 --source local --path ./libs/mylib` |

Planned future commands:

* `remove` — remove a package from the local index
* `build` — build registered packages
* `publish` — push packages to a remote or vendor store

---

## Design Notes

* **Header-only architecture:** all command registration happens through static initialization.
* **Zero-modification principle:** adding new commands does not require editing `main.cpp`.
* **Thread-safety:** protected by `std::mutex` in `CommandRegistry`.
* **C++20 features:** structured bindings, `std::filesystem`, lambdas, and `std::unique_ptr`.
* **Extensible context:** global options (e.g. `--config`, `--verbose`) can be later passed into commands via a shared `Context`.

---

## File Structure

```
CLIParse/
├─ CMakeLists.txt                # CMake configuration for CLI
├─ src/
│  └─ main.cpp                   # Entry point and CLI initialization
├─ include/localpm/cli/
│  ├─ registry.hpp               # CommandRegistry and Registrar
│  ├─ commands/
│  │  ├─ add.hpp                 # AddCommand definition
│  │  ├─ init.hpp                # InitCommand definition
│  │  └─ ...                     # Future commands
│  └─ commands_all.hpp           # Aggregates all commands for registration
└─ external/CLI11/               # Header-only dependency
```

---

## Dependencies

* **CLI11** — header-only command-line parser
* **C++20 STL** — uses `std::filesystem`, `std::unordered_map`, `std::unique_ptr`, etc.
* No runtime dependencies — static, portable binary.

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
```

---

## Project-Specific Notes

* The CLI parser is in **prototype stage** and currently operates standalone.
* Integration with the lockfile and backend package management logic is planned.
* Adding commands only requires creating a header in `commands/` and including it in `commands_all.hpp`.
* Future improvements will introduce a shared configuration context and integration with `localpm lockfile`.

---
