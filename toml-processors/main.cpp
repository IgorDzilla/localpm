#include "toml.hpp"

int main() {
	using toml_processors::Lockfile;

	Lockfile lf("sample.toml");
	lf.parse();
	lf.debug_print();
}
