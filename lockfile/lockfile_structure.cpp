#include "lockfile_structure.hpp"

#include <optional>
#include <string>
#include <vector>

/* Default values for quick constructions */
const std::string default_str = std::string{};
const std::vector<std::string> default_str_vec = std::vector<std::string>{};

Compiler::Compiler()
	: cc(default_str), cflags(std::nullopt), ldflags(std::nullopt) {}

Project::Project()
	: name(default_str), version(default_str), compiler(std::nullopt) {}

LocalSource::LocalSource() : path(default_str) {}

Integrity::Integrity() : tarball_sha256(std::nullopt) {}

Dependency::Dependency()
	: name(default_str), constraint(default_str), resolved(std::nullopt) {};

Package::Package()
	: id(default_str), name(default_str), version(default_str),
	  source(std::nullopt), integrity(std::nullopt),
	  dependencies(std::nullopt) {}

Lockfile::Lockfile() : project(Project()), packages(std::nullopt) {}
