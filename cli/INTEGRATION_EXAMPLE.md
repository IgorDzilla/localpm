# Using Lockfile from CLI Commands

## Example: Reading lockfile in a command

```cpp
#pragma once
#include "registry.hpp"
#include <CLI/CLI.hpp>
#include <iostream>

// Include lockfile library (available because cli links against lockfile)
#include "lockfile.hpp"

namespace localpm::cli {

class StatusCommand : public Command {
public:
    std::string name() const override { return "status"; }
    std::string description() const override {
        return "Show current project status from lockfile";
    }
    
    void configure(CLI::App &sub) override {
        sub.add_option("-f,--file", lockfile_path_, "Path to lockfile")
            ->default_val("localpm.lock");
    }
    
    int run() override {
        try {
            // Parse the lockfile
            LockfileParser parser(lockfile_path_);
            
            // Get and display project info
            const auto& project = parser.get_project();
            std::cout << "Project: " << project.name 
                      << " v" << project.version << "\n";
            
            // Display compiler if configured
            try {
                const auto& compiler = parser.get_compiler();
                std::cout << "Compiler: " << compiler.cc << "\n";
            } catch(...) {
                std::cout << "Compiler: not configured\n";
            }
            
            // Display packages
            const auto& packages = parser.get_packages();
            std::cout << "\nPackages (" << packages.size() << "):\n";
            for (const auto& pkg : packages) {
                std::cout << "  - " << pkg.name << "@" << pkg.version;
                
                if (pkg.type) {
                    std::cout << " [" << src_type_to_string(*pkg.type) << "]";
                }
                
                std::cout << "\n";
                
                // Show dependencies if any
                if (pkg.dependencies && !pkg.dependencies->empty()) {
                    std::cout << "    Dependencies:\n";
                    for (const auto& dep : *pkg.dependencies) {
                        std::cout << "      " << dep.name << " " << dep.constraint;
                        if (dep.resolved) {
                            std::cout << " (resolved: " << *dep.resolved << ")";
                        }
                        std::cout << "\n";
                    }
                }
            }
            
            return 0;
            
        } catch (const LockfileError& e) {
            std::cerr << "Lockfile error: " << e.what() << "\n";
            
            switch(e.code()) {
                case LockfileErrorCode::FILE_NOT_FOUND:
                    std::cerr << "Try running 'localpm init' first\n";
                    break;
                case LockfileErrorCode::TOML_PARSE_ERROR:
                    std::cerr << "Invalid lockfile format\n";
                    break;
                default:
                    break;
            }
            
            return 1;
        }
    }
    
private:
    std::string lockfile_path_;
};

} // namespace localpm::cli

// Auto-register the command
inline const bool registered_status =
    localpm::cli::CommandRegistry::instance()
        .register_type<localpm::cli::StatusCommand>();
```

## To add this command:

1. Save as `cli/include/commands/status.hpp`
2. Add to `cli/include/commands_all.hpp`:
   ```cpp
   #include "commands/status.hpp"
   ```
3. Rebuild - no CMake changes needed!

## Usage:

```bash
localpm status
localpm status --file path/to/custom.lock
```

## How it works:

1. CLI module's CMakeLists automatically links against `lockfile` if available
2. Command includes `lockfile.hpp` 
3. Command can use all lockfile parser functionality
4. Error handling provides user-friendly messages
