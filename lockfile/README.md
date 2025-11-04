# Lockfile Module

TOML-based lockfile parser for package management.

## Architecture

- **Library**: Compiled as a static/shared library
- **Headers**: `lockfile.hpp`, `lockfile_structure.hpp`
- **Dependency**: tomlplusplus (header-only TOML parser)
- **Language**: C++17

## Usage as Library

Link against the `lockfile` target:

```cmake
target_link_libraries(your_app PRIVATE lockfile)
```

In your code:

```cpp
#include "lockfile.hpp"

int main() {
    try {
        LockfileParser parser("path/to/lockfile.toml");
        
        // Get project info
        const auto& project = parser.get_project();
        std::cout << "Project: " << project.name << " v" << project.version << "\n";
        
        // Get packages
        const auto& packages = parser.get_packages();
        for (const auto& pkg : packages) {
            std::cout << pkg.name << "@" << pkg.version << "\n";
        }
        
        // Get compiler settings
        if (auto compiler = parser.get_compiler()) {
            std::cout << "Compiler: " << compiler.cc << "\n";
        }
        
    } catch (const LockfileError& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
```

## Data Structures

### Main Types

- **`Lockfile`**: Root structure containing schema, project, and packages
- **`Project`**: Project metadata (name, version, compiler)
- **`Package`**: Package information (name, version, source, dependencies)
- **`Compiler`**: Compiler configuration (cc, cflags, ldflags)
- **`Dependency`**: Package dependency with version constraint
- **`Source`**: Package source location (currently only `LocalSource`)
- **`Integrity`**: Checksums for package verification

### Enums

- **`LibKind`**: Library type (AutoDefined, HeaderOnly, Static, Shared, Abi)
- **`SrcType`**: Source type (Local, Git, Registry, Archive, Folder)
- **`LockfileErrorCode`**: Error codes for exception handling

## Error Handling

```cpp
try {
    LockfileParser parser("lockfile.toml");
} catch (const LockfileError& e) {
    switch(e.code()) {
        case LockfileErrorCode::FILE_NOT_FOUND:
            std::cerr << "File not found\n";
            break;
        case LockfileErrorCode::TOML_PARSE_ERROR:
            std::cerr << "Invalid TOML syntax\n";
            break;
        case LockfileErrorCode::FIELD_MISSING:
            std::cerr << "Required field missing\n";
            break;
        default:
            std::cerr << "Unknown error: " << e.what() << "\n";
    }
}
```

## Building Standalone

For testing the lockfile module independently:

```bash
mkdir build && cd build
cmake -DLOCKFILE_BUILD_STANDALONE=ON ..
cmake --build .
./lockfile_test
```

The `sample.toml` file is automatically copied to the build directory.

## Integration with CLI

The CLI module automatically links against `lockfile` if it's available. In your CLI commands:

```cpp
#include "lockfile.hpp"

class MyCommand : public Command {
    int run() override {
        LockfileParser parser("localpm.lock");
        // Use lockfile data in your command
        return 0;
    }
};
```

## Example Lockfile Format

```toml
schema = 1

[project]
name = "my-project"
version = "1.0.0"

[project.compiler]
cc = "gcc"
cflags = ["-Wall", "-O2"]
ldflags = ["-lpthread"]

[[packages]]
name = "fmt"
version = "10.2.1"
type = "local"
kind = "header-only"

[packages.source]
path = "./vendor/fmt"

[packages.integrity]
tarball_sha256 = "abc123..."

[[packages.dependencies]]
name = "some-dep"
version = "^1.0"
resolved = "1.2.3"
```

## Future Enhancements

- Support for Git sources
- Registry source support
- Archive/tarball sources
- More robust integrity checking

#Lockfile

`Lockfile` is core part of any project using `localpm`. It holds all information about packages and dependencies required to build the project. File format is `.toml` for convenience and simplicity of editing and reading.

## Lockfile structure

`sample.toml`
```
[lockfile]
schema = 1

[project]
name = "demo"
version = "0.1.0"

[project.compiler]
cc="gcc-14"
cflags=["-O3","-Wall"]
ldflags=["-s"]

[[packages]]
name="fmt"
version="10.2.1"
kind = "static"
type = "local"
source = {path="some/path/to/lib"}
integrity={ tarball_sha256="9f..cd" }

[[packages]]
name="demo"
version="0.1.0"
kind = "shared"
type = "local"
dependencies=[ { name="fmt", version="^10.2", resolved="10.2.1" } ]

[[packages]]
name="some-lib"
version="latest"
type = "local"
kind = "header-only"

```

## `[lockfile]`
Duty table. Will be automatically generated under any circumstances.

### `[project]`
Basic project information.

| Field   | Description            | Type   | Status                                  |
| ------- | ---------------------- | ------ | --------------------------------------- |
| name    | Project name           | string | Yes                                     |
| version | Version of the project | string | If not provided, will be set to "0.1.0" |

### `[project].compiler`
Information about compiler, build system and user required compile and linker flags.

| Field        | Description          | Type             | Status    |
| ------------ | -------------------- | ---------------- | --------- |
| cc           | Compiler name        | string           | Mandatory |
| cflags       | Compile flags        | array of strings | Optional  |
| ldflags      | Linker flags         | array of string  | Optional  |
| build_system | Name of build system | string           | Optional  |

> [!INFO] Build systems
> Currently no build systems are supported, only basic compilation with `cc`.
> Goal is to make `localpm` compatible with `cmake`.


### `[[packages]]`
Array of tables. Each holds info about project.

| Field        | Description                 | Type                       | Status                                     |
| ------------ | --------------------------- | -------------------------- | ------------------------------------------ |
| name         | Package name                | string                     | Mandatory                                  |
| version      | Version of the package      | string                     | Mandatory                                  |
| kind         | Kind of package             | string: one of given kinds | Optional *                                 |
| type         | Package's source type       | string: one of given types | Optional *                                 |
| dependencies | Required dependencies       | array of inline tables     | Optional                                   |
| source       | Info about package's source | inline table               | Mandatory unless type is local or registry |
| Integrity    | Package's integrity         | inline table               | Optional                                   |
#### `[packages.*].dependencies`
| Field    | Description | Type   | Status    |
| -------- | ----------- | ------ | --------- |
| name     | name        | string | Mandatory |
| version  | version     | string | Mandatory |
| resolved | -           | string | Optional  |
#### `[packages.*].source`
Currently empty.

#### `[packages.*].integrity`
| Field          | Description | Type   | Status    |
| -------------- | ----------- | ------ | --------- |
| tarball_sha256 | Cheksum     | string | Mandatory |


## Ways of generating lockfile
### Automatic
Run CLI commands generate `locfile.toml`. 

Running `localpm init` will generate basic template.
```
localpm init [args]
	[args]
		--template="<template name>" # from template
		--schema=<schema number>     # from basic shemas
		
```

Add packages:
```
localpm add <package_name>@<version> [args] # adds package with given name
	[args]
		--source=git/archive/fodler/registry
```
More info about CLI commands you can get in corresponding page of docs.

### Manual
You can make these files yourself. In an order to verify `lockfile.toml` you can use this command:
```
localpm checkheath --lockfile
```

If any errors exist, output will tell you.
