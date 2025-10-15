#pragma once

#include "tomlplusplus/toml.hpp"
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <string>

#include "lockfile_structure.hpp"

enum class LockfileErrorCode {
	SUCCESS = 0,
	UNKNOWN_ERROR = 1,
	FILE_NOT_FOUND = 2,
	TOML_PARSE_ERROR = 3,
	FIELD_MISSING = 4,
	TABLE_MISSING = 5,
	INVALID_VALUE = 6
};

class LockfileError : public std::exception {
  private:
	std::string msg_;
	LockfileErrorCode code_;

  public:
	LockfileError(std::string msg,
				  LockfileErrorCode code = LockfileErrorCode::UNKNOWN_ERROR)
		: msg_(msg), code_(code) {}
	LockfileError(const char *msg,
				  LockfileErrorCode code = LockfileErrorCode::UNKNOWN_ERROR)
		: msg_(std::string(msg)), code_(code) {}
	const char *what() const noexcept override { return msg_.c_str(); }
	const LockfileErrorCode code() const noexcept { return code_; }
};

class LockfileParser {
  private:
	std::string filepath_;
	toml::table tbl_;

	Lockfile lockfile_ = Lockfile();

	std::optional<Compiler> parse_compiler();
	Project parse_project();
	std::optional<std::unordered_map<std::string, Package>> parse_packages();
	std::optional<Source> parse_source(toml::table, SrcType);
	std::optional<Integrity> parse_integrity(toml::table);
	std::optional<Dependency> parse_dependecy(toml::table);
	void parse();

  public:
	LockfileParser(std::string filepath_);

	const std::vector<Package> &get_packages();
	const Compiler &get_compiler();
	const Project &get_project();
	const std::int64_t get_schema() const noexcept;
	void debug_print();
};
