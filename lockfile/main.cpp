#include "lockfile.hpp"

int main(void) {
	std::cout << "TEST IN PROGRESS\n";
	LockfileParser parser("sample.toml");

	std::cout << parser.get_schema() << std::endl;

	return 0;
}
