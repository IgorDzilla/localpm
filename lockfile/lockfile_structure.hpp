/*
 * INFO: This file contains structure of lockfile.toml.
 * All structures must be defined with default values to prevent incorrect
 * memory reading.
 *
 * */

#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Compiler {
	std::string cc = {};
	std::optional<std::vector<std::string>> cflags = std::nullopt;
	std::optional<std::vector<std::string>> ldflags = std::nullopt;

	explicit Compiler() = default;
};

struct Project {
	std::string name = {};
	std::string version = {};
	std::optional<Compiler> compiler = std::nullopt;

	explicit Project() = default;
};

/* Package sources */
// This variant will be implimented much later
// struct RegistrySource {
//   std::string url;     // https://registry.localpm.io
//   std::string package; // fmt
//   std::string rev;     // 10.2.1
// };
//
// struct GitSource {
// 	std::string url; // git url
// 	std::string rev; // commit/tag
// 	std::optional<std::string> subdir;
// };

struct LocalSource {
	std::string path = {}; // ./vendor/fmt

	explicit LocalSource() = default;
};

using Source = std::variant<LocalSource>;

struct Integrity {
	std::optional<std::string> tarball_sha256 = std::nullopt;

	explicit Integrity() = default;
};

// зависимость внутри пакета
struct Dependency {
	std::string name = {};								// fmt
	std::string constraint = {};						// ^10.2
	std::optional<std::string> resolved = std::nullopt; // 10.2.1

	explicit Dependency() = default;
};

enum class LibKind {
	Undefined = -1,
	HeaderOnly = 0,
	Static = 1,
	Shared = 2,
	Abi = 3
};

enum class SrcType { Undefined, Local, Git, Vendor, Archive };

struct Package {
	std::string name = {};	  // "fmt"
	std::string version = {}; // "10.2.1"
	SrcType type = SrcType::Undefined;
	LibKind kind = LibKind::Undefined;
	std::optional<Source> source = std::nullopt;
	std::optional<Integrity> integrity = std::nullopt;
	std::optional<std::vector<Dependency>> dependencies = std::nullopt;

	explicit Package() = default;
};

// Got a class that parses and holds everything in place. Do not know if this
// struct is needed.
struct Lockfile {
	std::int64_t schema = 1;
	Project project = Project();
	// packages по ключу секции ("fmt@10.2.1", "demo@0.1.0" и т.п.)
	std::optional<std::unordered_map<std::string, Package>> packages =
		std::nullopt;

	explicit Lockfile() = default;
};

/* Auxiliary functions */
inline const char *lib_kind_to_string(LibKind kind) {
	switch (kind) {
	case LibKind::HeaderOnly:
		return "header-only";
	case LibKind::Static:
		return "static";
	case LibKind::Shared:
		return "shared";
	case LibKind::Abi:
		return "abi";
	default:
		throw std::runtime_error("Uknown LibKind");
	}
}

inline const char *src_type_to_string(SrcType type) {
	switch (type) {
	case SrcType::Local:
		return "local";
	case SrcType::Git:
		return "git";
	case SrcType::Vendor:
		return "vendor";
	case SrcType::Archive:
		return "archive";
	default:
		throw std::runtime_error("Uknown SrcType");
	}
}
