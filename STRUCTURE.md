# Project Structure

## Directory Layout

```
localpm/
├── CMakeLists.txt           # Root CMake configuration
├── .gitignore               # Git ignore rules
├── README.md                # Project README
├── STRUCTURE.md             # This file
│
├── cli/                     # CLI module (from scratches branch)
│   ├── CMakeLists.txt       # CLI library configuration
│   ├── include/             # CLI headers
│   │   ├── commands/        # Command implementations
│   │   │   ├── add.hpp
│   │   │   ├── init.hpp
│   │   │   ├── install.hpp
│   │   │   └── list.hpp
│   │   ├── commands_all.hpp
│   │   └── registry.hpp
│   └── src/                 # CLI source files
│
├── lockfile/                # Lockfile parser module (from scratches branch)
│   ├── CMakeLists.txt       # Lockfile library configuration
│   ├── lockfile.hpp         # Lockfile parser header
│   ├── lockfile.cpp         # Lockfile parser implementation
│   ├── lockfile_structure.hpp
│   └── sample.toml          # Example lockfile
│
├── filesystem/              # Filesystem/database module
│   ├── CMakeLists.txt       # Filesystem module configuration
│   ├── src/                 # Filesystem source files
│   │   └── storage/         # Database/storage implementations
│   │       ├── schema.hpp
│   │       └── schema.cpp
│   └── SQLiteCpp/           # SQLite dependency (submodule or vendored)
│
├── src/                     # Core library (optional)
│   ├── CMakeLists.txt       # Core library configuration
│   └── include/             # Core library headers
│
├── apps/                    # Executable applications
│   ├── CMakeLists.txt       # Apps configuration
│   └── main.cpp             # Main executable entry point
│
└── tests/                   # Unit tests
    ├── CMakeLists.txt       # Test configuration
    └── test_*.cpp           # Test files
```

## CMake Structure

### Root CMakeLists.txt
- Sets project-wide configuration (C++20, compile commands export)
- Conditionally includes subdirectories based on existence
- Provides options: `LOCALPM_BUILD_TESTS`, `LOCALPM_BUILD_APPS`

### Module Libraries
- **cli**: Interface library exposing CLI commands and registry
- **lockfile**: Static/shared library for parsing TOML lockfiles
- **filesystem**: (Can be added) Database and filesystem operations

### Executable
- **localpm**: Main executable that links against `cli` and `lockfile`

## Building

### Standard build:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Build options:
```bash
# Disable tests
cmake -DLOCALPM_BUILD_TESTS=OFF ..

# Disable apps
cmake -DLOCALPM_BUILD_APPS=OFF ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Merging from scratches branch

To bring in cli and lockfile modules:
```bash
# Option 1: Cherry-pick specific directories
git checkout scratches -- cli/ lockfile/

# Option 2: Merge entire branch
git merge scratches

# Option 3: Copy files manually
```

## Dependencies

- **CLI11**: Fetched via CMake FetchContent (for CLI parsing)
- **tomlplusplus**: Fetched via CMake FetchContent (for TOML parsing)
- **SQLiteCpp**: Fetched via CMake FetchContent or git submodule
