#pragma once

#include <filesystem>
#include <semver/semver.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

namespace localpm::file_process {

namespace fs = std::filesystem;

// --------- Ошибки ---------

class NotDirError : public std::runtime_error {
  public:
	explicit NotDirError(const std::string &p)
		: std::runtime_error("Path \"" + p +
							 "\" already exists, but is not a directory.") {}
};

class PackageVersionExistsError : public std::runtime_error {
  public:
	PackageVersionExistsError(std::string ns, std::string name,
							  std::string version)
		: std::runtime_error("Package version already exists: " + ns +
							 "::" + name + "@" + version),
		  ns_(std::move(ns)), name_(std::move(name)),
		  version_(std::move(version)) {}

	const std::string &ns() const noexcept { return ns_; }
	const std::string &name() const noexcept { return name_; }
	const std::string &version() const noexcept { return version_; }

  private:
	std::string ns_;
	std::string name_;
	std::string version_;
};

class SemVerParseError : public std::runtime_error {
  public:
	SemVerParseError(std::string ver, std::string msg)
		: std::runtime_error("Invalid SemVer '" + ver + "': " + msg),
		  ver_(std::move(ver)) {}

	const std::string &version() const noexcept { return ver_; }

  private:
	std::string ver_;
};

// --------- Помощники ---------

bool is_valid_ident(std::string_view s);

// --------- Layout store'а ---------

struct StorageLayout {
	fs::path root;
	fs::path config;
	fs::path logs;
	fs::path cache;
	fs::path index_dir;
	fs::path index_db;
	fs::path packages;

	explicit StorageLayout(fs::path root);
};

struct PackageLayout {
	fs::path ns_dir;
	fs::path pkg_dir;
	fs::path ver_dir;
	fs::path manifest;
	fs::path source_dir;
	fs::path build_dir;
	fs::path meta_json;
	fs::path latest_link;

	PackageLayout(const StorageLayout &sl, std::string_view ns,
				  std::string_view name, std::string_view version);
};

// --------- API ---------

/*
 * Checks if path points to directory.
 * If path exists but is not a directory -> throws NotDirError.
 * If path does not exist -> creates all needed directories.
 */
void ensure_dir(const fs::path &path);

/*
 * Initialize storage layout at given root.
 * Creates basic directories and (если надо) config/index.db.
 */
void init_storage(const fs::path &root);

/*
 * Ensure that given package version exists on disk:
 *  - validates ns/name
 *  - parses & нормализует SemVer (через cpp-semver)
 *  - создаёт каталоги и manifest.toml
 *  - пересчитывает latest -> на максимальную stable SemVer
 */
void ensure_package_version(const StorageLayout &sl, std::string_view ns,
							std::string_view name, std::string_view version_str,
							std::string_view manifest_content);

/*
 * Пересчитать симлинк latest для пакета.
 * Если stable_only = true — берём только stable версии (без prerelease).
 */
void update_latest_symlink(const PackageLayout &pl, bool stable_only = true);

/*
 * Копирует папку с новой версией в соответсвуюшее место в древе
 * */
void copy_package_version(fs::path &path);

void import_package_version(const StorageLayout &sl, std::string_view ns,
							std::string_view name, const fs::path &src_ver_dir,
							std::string version = {});
} // namespace localpm::file_process
