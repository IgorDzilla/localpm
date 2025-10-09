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
		throw LockfileError("Essential table [project] is missng.",
							LockfileErrorCode::FIELD_MISSING);

	Project project;

	if (auto s = node["name"].value<std::string>()) {
		project.name = std::move(*s);
	} else {
		throw LockfileError(
			"Essential field `name` is missing in table `project`.",
			LockfileErrorCode::FIELD_MISSING);
	}

	project.version = node["version"].value_or(std::string{});

	project.compiler =
		std::move(*parse_compiler()); // returns a value or nullopt, so no
									  // need in additional checks

	return project;
}

std::optional<std::unordered_map<std::string, Package>>
LockfileParser::parse_packages() {
	auto *pkg_table = tbl_["packages"].as_table();
	if (!pkg_table)
		return std::nullopt;
	else if (!pkg_table || !pkg_table->as_table())
		throw LockfileError("[packages] must be a table",
							LockfileErrorCode::INVALID_VALUE);

	std::unordered_map<std::string, Package> packages;

	for (auto &&[k, node] : *pkg_table) {

		std::string id = std::string(k.str());

		auto *pt = node.as_table();
		if (!pt)
			throw LockfileError("Each [packages.\"...\"] must be a table.",
								LockfileErrorCode::INVALID_VALUE);

		// add auto parse from key later
		auto name = pt->at_path("name").value_or(std::string{});
		auto version = pt->at_path("version").value_or(std::string{});
		// source = { ... }  (inline-table)
		std::optional<Source> source;
		if (auto s = pt->get("source")) {
			auto *st = s->as_table();
			if (!st)
				throw std::runtime_error("package.source must be a table");

			auto type = st->at_path("type").value_or(std::string{});

			if (type == "local") {
				LocalSource ls{.path =
								   st->at_path("path").value_or(std::string{})};
				source = std::move(ls);
			} else if (!type.empty()) {
				throw std::runtime_error("Unknown source.type: " + type);
			}
		}

		// integrity = { tarball_sha256="..." }
		std::optional<Integrity> integrity;
		if (auto in = pt->get("integrity")) {
			auto *it = in->as_table();
			if (!it)
				throw std::runtime_error("package.integrity must be a table");
			Integrity integ;
			if (auto h = it->at_path("tarball_sha256").value<std::string>())
				integ.tarball_sha256 = *h;
			integrity = std::move(integ);
		}

		// dependencies = [ { ... }, { ... } ]
		std::optional<std::vector<Dependency>> deps = std::nullopt;
		if (auto dn = pt->get("dependencies")) {
			auto *arr = dn->as_array();
			if (!arr)
				throw std::runtime_error(
					"package.dependencies must be an array");
			for (auto &&item : *arr) {
				auto *dt = item.as_table();
				if (!dt)
					throw std::runtime_error(
						"dependency entry must be a table");
				Dependency d;
				d.name = dt->at_path("name").value_or(std::string{});
				d.constraint =
					dt->at_path("constraint").value_or(std::string{});
				if (auto r = dt->at_path("resolved").value<std::string>())
					d.resolved = *r;
				deps->push_back(std::move(d));
			}
		}

		// Сложи в свою структуру Package и положи куда нужно (map, вектор и
		// т.п.)
		Package p;
		p.id = id;
		p.name = std::move(name);
		p.version = std::move(version);
		p.source = std::move(source);
		p.integrity = std::move(integrity);
		p.dependencies = std::move(deps);

		packages[id] = p;
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

void LockfileParser::debug_print() {
	std::cout << "[lockfile]" << std::endl;
	std::cout << "\t" << lockfile_.schema << std::endl;
	std::cout << "\n[project]\n";
	std::cout << "\t" << lockfile_.project.name << "\n";
	std::cout << "\t" << lockfile_.project.version << "\n";
	if (lockfile_.project.compiler) {
		std::cout << "\t[project.compiler]\n";
		std::cout << "\t\t" << lockfile_.project.compiler->cc << std::endl;
		for (auto &s : lockfile_.project.compiler->cflags)
			std::cout << "\t\t" << s << std::endl;
		for (auto &s : lockfile_.project.compiler->ldflags)
			std::cout << "\t\t" << s << std::endl;
	}

	if (lockfile_.packages) {
		for (auto &&pair : lockfile_.packages.value()) {
			std::cout << "[packages." << pair.first << "]\n";
			std::cout << "\t\t" << pair.second.name << "\n";
			std::cout << "\t\t" << pair.second.id << "\n";
			std::cout << "\t\t" << pair.second.version << "\n";
			if (pair.second.dependencies) {
				std::cout << "\t\t[dependencies]\n";
				for (auto &dep : pair.second.dependencies.value()) {
					std::cout << "\t\t\t" << dep.name << std::endl;
					std::cout << "\t\t\t" << dep.constraint << std::endl;
				}
			}
		}
	}
}
