# Lockfile Module - Quick Start

## Build & Test

### Test lockfile module alone:
```bash
cd lockfile
mkdir build && cd build
cmake -DLOCKFILE_BUILD_STANDALONE=ON ..
cmake --build .
./lockfile_test
```

### Build as part of main project:
```bash
cd ..  # back to project root
mkdir build && cd build
cmake ..
cmake --build .
```

## Using in CLI Commands

Just include the header - linking is automatic:

```cpp
#include "lockfile.hpp"

class MyCommand : public Command {
    int run() override {
        LockfileParser parser("localpm.lock");
        const auto& project = parser.get_project();
        std::cout << project.name << "\n";
        return 0;
    }
};
```

See [INTEGRATION_EXAMPLE.md](../cli/INTEGRATION_EXAMPLE.md) for a complete example.

## Common Operations

### Parse lockfile:
```cpp
LockfileParser parser("path/to/lockfile.toml");
```

### Get project info:
```cpp
const auto& project = parser.get_project();
std::cout << project.name << " v" << project.version;
```

### Get all packages:
```cpp
const auto& packages = parser.get_packages();
for (const auto& pkg : packages) {
    std::cout << pkg.name << "@" << pkg.version << "\n";
}
```

### Get compiler settings:
```cpp
try {
    const auto& compiler = parser.get_compiler();
    std::cout << "CC: " << compiler.cc << "\n";
} catch(...) {
    // No compiler configured
}
```

### Error handling:
```cpp
try {
    LockfileParser parser("file.toml");
} catch (const LockfileError& e) {
    std::cerr << e.what() << "\n";
    // e.code() gives LockfileErrorCode enum
}
```

## CMake Integration

The lockfile library:
- ✅ Automatically fetches tomlplusplus
- ✅ Exposes PUBLIC headers
- ✅ Automatically linked by CLI module
- ✅ Can be used standalone

No manual dependency management needed!
