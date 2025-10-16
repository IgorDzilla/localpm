# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

`localpm` is a simple package manager for C/C++ projects, inspired by pip. This is currently a prototype/research project with components being developed in the `lockfile/` directory.

## Development Commands

### Building
This project uses a simple compilation approach without a build system like Make or CMake:

```bash
# Navigate to the lockfile directory
cd lockfile

# Compile with debug symbols and sanitizers (current configuration)
clang++ -std=c++17 -g -fsanitize=address -I./tomlplusplus/include main.cpp lockfile.cpp -o main

# Compile optimized version 
clang++ -std=c++17 -O2 -I./tomlplusplus/include main.cpp lockfile.cpp -o main

# Run the compiled program
./main
```

### Code Formatting
The project uses clang-format for code formatting:

```bash
# Format all C++ files in lockfile directory
cd lockfile
clang-format -i *.cpp *.hpp
```

### Testing
Currently there are no automated tests. The project uses a simple main.cpp that manually tests the lockfile parser with sample.toml.

## Architecture

### Core Components

**LockfileParser** (`lockfile.hpp`, `lockfile.cpp`)
- Main parsing engine for TOML lockfiles
- Uses toml++ library for TOML parsing
- Handles parsing of project metadata, compiler settings, and package dependencies
- Implements comprehensive error handling with custom error codes

**Data Structures** (`lockfile_structure.hpp`)
- Defines all data structures for the lockfile format:
  - `Project`: Project metadata (name, version, compiler settings)
  - `Package`: Package information (name, version, source, dependencies)
  - `Compiler`: Compiler configuration (cc, cflags, ldflags)
  - `Source`: Package source definitions (currently only LocalSource implemented)
  - `Dependency`: Package dependency relationships
  - `Lockfile`: Root container for all lockfile data

### Lockfile Format
The project defines a TOML-based lockfile format similar to other package managers:
- Schema versioning support
- Project metadata section
- Compiler configuration 
- Package array with dependency resolution
- Support for different package types (local, git, registry - future)
- Package kinds: header-only, static, shared, abi

### Current State
This is a prototype in the "scratches" branch. The lockfile parser is implemented but has some memory management issues (AddressSanitizer errors in vector operations). The architecture is designed for future expansion to support multiple source types beyond local packages.

### Dependencies
- **toml++**: Header-only TOML parsing library (included in `lockfile/tomlplusplus/`)
- Standard C++17 features extensively used (variants, optionals, unordered_map)

## Project-Specific Notes

- The project is in early prototype stage - expect incomplete implementations
- Uses modern C++17 features heavily (std::variant, std::optional)
- Error handling uses custom exception types with specific error codes
- Memory safety issues exist in current implementation (AddressSanitizer failures)
- Architecture is designed for extensibility (Source variant can be expanded for git/registry sources)
- No formal testing framework yet - relies on manual testing with sample.toml

## File Structure
- `lockfile/` - Main development directory containing the lockfile parser prototype
- `lockfile/sample.toml` - Example lockfile for testing
- `lockfile/tomlplusplus/` - Vendored TOML parsing library