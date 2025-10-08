#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Compiler {
	std::string cc;
	std::vector<std::string> cflags;
	std::vector<std::string> ldflags;
};

struct Project {
	std::string name;
	std::string version;
	std::optional<Compiler> compiler;
};

/* Package sources */
// This variant will be implimented much later
// struct RegistrySource {
//   std::string url;     // https://registry.localpm.io
//   std::string package; // fmt
//   std::string rev;     // 10.2.1
// };

struct GitSource {
	std::string url; // git url
	std::string rev; // commit/tag
	std::optional<std::string> subdir;
};

struct LocalSource {
	std::string path; // ./vendor/fmt
};

using Source = std::variant<GitSource, LocalSource>;

struct Integrity {
	std::optional<std::string> tarball_sha256;
};

// зависимость внутри пакета
struct Dependency {
	std::string name;					 // fmt
	std::string constraint;				 // ^10.2
	std::optional<std::string> resolved; // 10.2.1
};

struct Package {
	std::string id;		 // ключ секции: "fmt@10.2.1" (для адресации)
	std::string name;	 // "fmt"
	std::string version; // "10.2.1"
	std::optional<Source> source;
	std::optional<Integrity> integrity;
	std::vector<Dependency> dependencies;
};

// Got a class that parses and holds everything in place. Do not know if this
// struct is needed.
struct Lockfile {
	std::int64_t schema = 1;
	Project project;
	// packages по ключу секции ("fmt@10.2.1", "demo@0.1.0" и т.п.)
	std::unordered_map<std::string, Package> packages;
};
