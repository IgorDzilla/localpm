/*
 * INFO: Public interface for working with manifest.toml
 *
 * ManifestProcessor отвечает за:
 *   - загрузку manifest.toml с диска;
 *   - парсинг в структуры localpm::manifest::Manifest;
 *   - сохранение изменений обратно в TOML.
 */

#pragma once

#include "manifest/manifest_structure.hpp"
#include <filesystem>

namespace localpm::filesys {
class ManifestProcessor {
  public:
	// path — manifest.toml
	explicit ManifestProcessor(const std::filesystem::path &path);

	// access data in memory
	const manifest::Manifest &data() const noexcept { return manifest_; }
	manifest::Manifest &data() noexcept { return manifest_; }

	// операции с файлом manifest.toml
	void reload();	   // rewrire from disk
	void save() const; // save on disk

	// get path
	const std::filesystem::path &path() const noexcept { return path_; }

  private:
	std::filesystem::path path_{}; // path to toml manifest
	manifest::Manifest manifest_{};
};

// Удобные свободные функции
manifest::Manifest load_manifest(const std::filesystem::path &path);
void write_manifest(const manifest::Manifest &m,
					const std::filesyste m::path &path);

} // namespace localpm::filesys
