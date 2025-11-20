/*
 * INFO: This file contains structure of manifest.toml.
 * All structures must be defined with default values to prevent incorrect
 * memory reading.
 *
 * manifest.toml описывает намерения проекта:
 *   - базовую информацию о пакете;
 *   - список зависимостей (из registry/git/path/archive);
 *   - dev-/build-зависимости.
 *
 * В отличие от lockfile.toml, здесь нет, checksum и т.п.:
 * только то, что написал пользователь.
 */

#pragma once

#include <optional>
#include <string>
#include <toml++/toml.hpp>
#include <vector>

namespace localpm::manifest {
// --------------------------
// [package]
// --------------------------
struct PackageInfo {
	std::string name{};	   // обязательное поле
	std::string version{}; // обязательное поле

	std::optional<std::string> description = std::nullopt;
	std::vector<std::string> authors{};
	std::optional<std::string> license = std::nullopt;
	std::optional<std::string> homepage = std::nullopt;
	std::optional<std::string> kind = std::nullopt;
};

/*
 * Описание одной зависимости из секций:
 *   [dependencies]
 *   [dev-dependencies]
 *   [build-dependencies]
 *
 * Покрываем источники из lockfile:
 *   - registry (по умолчанию, если только version)
 *   - git
 *   - path (локальный путь)
 *   - archive (tar.gz/zip и т.п.)
 */

// --------------------------
// Описание одной зависимости
// --------------------------
struct DependencyDecl {
	std::string name{};
	std::string version_spec{};

	// По умолчанию ВСЕ optional — nullopt
	std::optional<std::string> registry = std::nullopt;

	std::optional<std::string> git = std::nullopt;
	std::optional<std::string> tag = std::nullopt;
	std::optional<std::string> rev = std::nullopt;

	std::optional<std::string> path = std::nullopt;

	std::optional<std::string> archive = std::nullopt;
	std::optional<std::string> checksum = std::nullopt;
};

// --------------------------
// Полный manifest.toml
// --------------------------
struct Manifest {
	PackageInfo package{};

	std::vector<DependencyDecl> dependencies{};
	std::vector<DependencyDecl> dev_dependencies{};
	std::vector<DependencyDecl> build_dependencies{};
};

// --------------------------
// TOML <-> Manifest конвертер
// --------------------------
class ManifestTomlAdapter {
  public:
	static Manifest from_toml(const toml::table &root);
	static toml::table to_toml(const Manifest &manifest);
};

} // namespace localpm::manifest
