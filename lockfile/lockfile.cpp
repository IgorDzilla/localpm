#include "lockfile.hpp"
#include "lockfile_structure.hpp"
#include "tomlplusplus/toml.hpp"
#include <optional>
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

	Compiler compiler;

	if (auto s = node["cc"].value<std::string>())
		compiler.cc = std::move(*s);
	else {
		throw LockfileError("Field `cc` is missing in table `project.compiler`",
							LockfileErrorCode::FIELD_MISSING);
	}

	if (auto arr = node["cflags"].as_array()) {
		compiler.cflags.reserve(arr->size());
		for (auto &&n : *arr)
			if (auto s = n.value<std::string>())
				compiler.cflags.emplace_back(std::move(*s));
	}

	if (auto arr = node["ldflags"].as_array()) {
		compiler.ldflags.reserve(arr->size());
		for (auto &&n : *arr)
			if (auto s = n.value<std::string>())
				compiler.ldflags.emplace_back(std::move(*s));
	}

	return compiler;
}

/* Parse the project table */
Project LockfileParser::parse_project() {
	auto node = tbl_["project"];
	if (!node || !node.is_table())
		throw LockfileError("Essential table `project` is missng.",
							LockfileErrorCode::FIELD_MISSING);

	Project project;

	if (auto s = tbl_["name"].value<std::string>()) {
		project.name = std::move(*s);
	} else {
		throw LockfileError(
			"Essential field `name` is missing in table `project`.",
			LockfileErrorCode::FIELD_MISSING);
	}

	project.version = tbl_["version"].value_or(std::string{});

	project.compiler =
		std::move(*parse_compiler()); // returns a value or nullopt, so no
									  // need in additional checks

	return project;
}

std::optional<std::vector<Package>> LockfileParser::parse_packages() {
	auto *pkg_table = tbl_["packages"].as_table();
	if (!pkg_table)
		return std::nullopt;
	else if (!pkg_table || !pkg_table->as_table())
		throw LockfileError("[packages] must be a table",
							LockfileErrorCode::INVALID_VALUE);
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
}

const std::int64_t LockfileParser::get_schema() const noexcept {
	return lockfile_.schema;
}
