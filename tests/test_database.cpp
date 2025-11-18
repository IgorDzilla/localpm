#include "database.hpp"
#include <gtest/gtest.h>
#include <iostream>

TEST(Database, UpsertPackage) {
	std::string db_path = DB_PATH;
	localpm::database::DataBase db(db_path);

	std::cerr << "Creating db\n";
	db.init_db();

	std::cerr << "Upserts in progress...\n";
	localpm::database::Package pkg{};
	pkg.name = "libA";
	pkg.pkg_namespace = "ns";
	pkg.version = "0.1.0";
	pkg.path = "/x/libA/0.1.0";
	pkg.src_type = "vendor";
	pkg.pkg_type = "static-lib";

	db.upsert_package(pkg);

	pkg.name = "libA";
	pkg.pkg_namespace = "ns";
	pkg.version = "0.1.1";
	pkg.path = "/x/libb/0.1.1";
	pkg.src_type = "local";
	pkg.pkg_type = "static-lib";

	db.upsert_package(pkg);

	std::cerr << "Searching\n";
	auto pkgs = db.search_packages();

	for (auto p : pkgs) {
		std::cout << p.first << "@" << p.second.version << std::endl;
	}

	ASSERT_TRUE(true);
}
