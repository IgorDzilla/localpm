# localpm
Simple package manager for C/C++ inspired by pip.

## Quick Start

### Prerequisites
- CMake 3.16+
- C++20 capable compiler (GCC 10+, Clang 10+, MSVC 2019+)

### Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Executable will be in `build/bin/localpm`

### Merging modules from scratches branch

The `cli` and `lockfile` modules are in the `scratches` branch:

```bash
git checkout scratches -- cli/ lockfile/
```

Then rebuild:

```bash
cd build
cmake ..
cmake --build .
```

## Project Structure

See [STRUCTURE.md](STRUCTURE.md) for detailed documentation.

- `cli/` - Command-line interface module
- `lockfile/` - TOML lockfile parser
- `filesystem/` - Database and filesystem operations
- `apps/` - Main executable
- `tests/` - Unit tests
