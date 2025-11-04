#include "lockfile.hpp"
#include "lockfile_structure.hpp"
#include "tomlplusplus/toml.hpp"
#include <optional>
#include <unordered_map>
#include <vector>

LockfileParser::LockfileParser(std::string filepath) : filepath_(filepath) {
	try {
		tbl_ = toml::parse_file(filepath_);
	} catch (const toml::parse_error &err) {
		throw LockfileError(err.what(), LockfileErrorCode::TOML_PARSE_ERROR);
	}

	try {
		parse();
	} catch (const LockfileError &err) {
		throw err;
	}
}

std::optional<Compiler> LockfileParser::parse_compiler() {
	auto node = tbl_["project"]["compiler"];
	if (!node || !node.is_table()) {
		return std::nullopt;
	}

	Compiler compiler = Compiler();

	if (auto s = node["cc"].value<std::string>())
		compiler.cc = std::move(*s);
	else {
		throw LockfileError("Field `cc` is missing in table `project.compiler`",
							LockfileErrorCode::FIELD_MISSING);
	}

	if (auto arr = node["cflags"].as_array()) {
		compiler.cflags = std::vector<std::string>();
		compiler.cflags->reserve(arr->size());
		for (auto &&n : *arr)
			if (auto s = n.value<std::string>())
				compiler.cflags->emplace_back(std::move(*s));
	}

	if (auto arr = node["ldflags"].as_array()) {

		compiler.ldflags->reserve(arr->size());
		for (auto &&n : *arr)
			if (auto s = n.value<std::string>())
				compiler.ldflags->emplace_back(std::move(*s));
	}

	return compiler;
}

/* Parse the project table */
Project LockfileParser::parse_project() {
	auto node = tbl_["project"];
	if (!node || !node.is_table())
		throw LockfileError("Table [project] is missng.",
							LockfileErrorCode::FIELD_MISSING);

	Project project;

	if (auto s = node["name"].value<std::string>()) {
		project.name = std::move(*s);
	} else {
		throw LockfileError("Field [project].name is missng",
							LockfileErrorCode::FIELD_MISSING);
	}

	project.version = node["version"].value_or(std::string{"0.1.0"});

	project.compiler =
		std::move(*parse_compiler()); // returns a value or nullopt, so no
									  // need in additional checks

	return project;
}

Integrity LockfileParser::parse_integrity(toml::table int_tbl) {
	Integrity integrity;
	if (auto sha = int_tbl["tarball_sha256"]) {
		if (!sha.is_string())
			throw LockfileError(
				"Field [packages.*].integrity.tarball_sha256 must be a stirng.",
				LockfileErrorCode::INVALID_VALUE);

		integrity.tarball_sha256 = sha.value_or(std::string{});
	}

	return integrity;
}

Dependency LockfileParser::parse_dependency(toml::table dep_tbl) {
	Dependency dependency;

	auto name = dep_tbl["name"];
	if (!name) {
		throw LockfileError("Field [packages.*].[dependency].name is missing.",
							LockfileErrorCode::FIELD_MISSING);
	}
	if (!name.is_string()) {
		throw LockfileError(
			"Field [packages.*].[dependency].name must be a string.",
			LockfileErrorCode::INVALID_VALUE);
	}
	dependency.name = *name.value<std::string>();

	auto version = dep_tbl["version"];
	if (!version) {
		throw LockfileError(
			"Field [packages.*].[dependencies].version is missing",
			LockfileErrorCode::FIELD_MISSING);
	}
	if (!version.is_string()) {
		throw LockfileError(
			"Field [packages.*].[dependencies].version must be a string.",
			LockfileErrorCode::INVALID_VALUE);
	}

	if (auto resolved = dep_tbl["resolved"]) {
		if (!resolved.is_string()) {
			throw LockfileError(
				"Field [packages.*].[dependencies].resolved must be a string.",
				LockfileErrorCode::INVALID_VALUE);
		}

		dependency.resolved = resolved.value<std::string>();
	}

	return dependency;
}

// WARNING: Will be a router to functions fot parsing different types of
// Source Now only LocalSource is implimented
Source LockfileParser::parse_source(toml::table package_table, SrcType type) {
	return LocalSource();
}

static LibKind parse_kind_string(const std::string &s) {
	// std::cout << "\t->" << s << std::endl;
	if (s == "auto-defined")
		return LibKind::AutoDefined;
	if (s == "header-only")
		return LibKind::HeaderOnly;
	if (s == "static")
		return LibKind::Static;
	if (s == "shared")
		return LibKind::Shared;
	if (s == "abi")
		return LibKind::Abi;

	throw LockfileError("Uknown packages[].kind: " + s,
						LockfileErrorCode::INVALID_VALUE);
}

static SrcType parse_type_string(const std::string &s) {
	if (s == "local")
		return SrcType::Local;
	if (s == "git")
		return SrcType::Git;
	if (s == "registry")
		return SrcType::Regsirty;
	if (s == "archive")
		return SrcType::Archive;
	if (s == "folder")
		return SrcType::Folder;

	throw LockfileError("Uknown packages[].type: " + s,
						LockfileErrorCode::INVALID_VALUE);
}

std::optional<std::unordered_map<std::string, Package>>
LockfileParser::parse_packages() {
	auto *arr = tbl_["packages"].as_array();
	if (!arr) {
		return std::nullopt;
	}

	std::unordered_map<std::string, Package> packages;

	for (auto &&node : *arr) {
		const auto *package_table = node.as_table();
		if (!package_table->is_table()) {
			throw LockfileError("Each [package] must be a table.",
								LockfileErrorCode::INVALID_VALUE);
		}

		Package p;

		// Parsing simple fields of package
		auto name = (*package_table)["name"].value<std::string>();
		if (!name) {
			throw LockfileError("Field [packages.*].name is missing.",
								LockfileErrorCode::FIELD_MISSING);
		}
		p.name = *name;

		auto version = (*package_table)["version"].value<std::string>();
		if (!version) {
			throw LockfileError("Field [packages.*].version is missing.",
								LockfileErrorCode::FIELD_MISSING);
		}
		p.version = *version;

		auto type = (*package_table)["type"].value<std::string>();
		if (!type) {
			// throw LockfileError("Field [packages.*].type is missing");
			p.type = SrcType::Local;
		} else {
			p.type = parse_type_string(*type);
		}

		auto kind = (*package_table)["kind"].value<std::string>();
		if (!type) {
			throw LockfileError("Field [packages.*].kind is missing");
		} else if (p.type == SrcType::Local) {
			p.kind = LibKind::AutoDefined;
		} else {
			p.kind = parse_kind_string(*kind);
		}
		// end of parsing simple fields of package

		// Parsing inline table Source
		if (auto src_tbl = (*package_table)["source"]) {
			if (!src_tbl.is_table()) {
				throw LockfileError("[packages.*].source must be inline table.",
									LockfileErrorCode::INVALID_VALUE);
			}
			p.source = parse_source(*src_tbl.as_table(), *p.type);
		}

		// Parse inline table Integrity
		if (auto int_tbl = (*package_table)["integrity"]) {
			if (!int_tbl.is_table()) {
				throw LockfileError(
					"[packages.*].integrity must be inline table.",
					LockfileErrorCode::INVALID_VALUE);
			}

			p.integrity = parse_integrity(*int_tbl.as_table());
		}

		if (auto dep_tbls_arr = (*package_table)["dependencies"].as_array()) {
			p.dependencies = std::vector<Dependency>();
			for (auto &&dep_tbl : *dep_tbls_arr) {
				if (!dep_tbl.is_table()) {
					throw LockfileError(
						"Each [packages.*].dependencies must be a table",
						LockfileErrorCode::INVALID_VALUE);
				}

				p.dependencies->push_back(
					parse_dependency(*dep_tbl.as_table()));
			}
		}

		packages[p.name + "@" + p.version] = p;
	}

	return packages;
}

void LockfileParser::parse() {
	// parse lockfile table
	auto t = tbl_["lockfile"];
	if (!t.is_table()) {
		throw LockfileError(
			std::string("Essential table `lockfile` is missing."),
			LockfileErrorCode::TABLE_MISSING);
	}
	if (auto n = t["schema"].value<std::int64_t>())
		lockfile_.schema = *n;
	else
		throw LockfileError(
			std::string(
				"Essential field `schema` of table `lockfile` is missing."),
			LockfileErrorCode::FIELD_MISSING);

	lockfile_.packages = parse_packages();
	lockfile_.project = parse_project();
}

const std::int64_t LockfileParser::get_schema() const noexcept {
	return lockfile_.schema;
}
