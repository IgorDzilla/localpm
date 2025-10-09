#include "lockfile.hpp"

int main(void) {
	std::cout << "TEST IN PROGRESS\n";
	LockfileParser parser("sample.toml");

	parser.debug_print();

	return 0;
}
