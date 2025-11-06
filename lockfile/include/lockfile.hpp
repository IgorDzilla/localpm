#pragma once

#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <toml++/toml.hpp>

#include "lockfile_structure.hpp"

#ifndef SCHEMA_DIR
#define SCHEMA_DIR "schemas/"
#endif

extern const std::string schema_template;

namespace localpm::filesys {
enum class LockfileErrorCode {
	SUCCESS = 0,
	UNKNOWN_ERROR = 1,
	FILE_NOT_FOUND = 2,
	TOML_PARSE_ERROR = 3,
	FIELD_MISSING = 4,
	TABLE_MISSING = 5,
	INVALID_VALUE = 6,
	FILE_ERROR = 7,
	INVAL_SCHEMA = 9,
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

class LockfileProcessor {
  private:
	std::string filepath_;
	toml::table tbl_;
	bool parsed_ = false;
	size_t schema_;

	Lockfile lockfile_ = Lockfile();

	std::optional<Compiler> parse_compiler();
	Project parse_project();
	std::optional<std::unordered_map<std::string, Package>> parse_packages();
	Source parse_source(toml::table, SrcType);
	Integrity parse_integrity(toml::table);
	Dependency parse_dependency(toml::table);
	void get_table_from_file();
	void write_file(std::string); // writes given string to filepathe_

  public:
	LockfileProcessor(std::string &filepath_, size_t schema = 0);

	void parse();
	void write_template();

	const std::vector<Package> &get_packages();
	const Compiler &get_compiler();
	const Project &get_project();
	const std::int64_t get_schema() const noexcept;
	void debug_print();
};
} // namespace localpm::filesys
