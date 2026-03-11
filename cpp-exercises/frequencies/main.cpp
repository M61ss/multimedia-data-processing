#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <ostream>

int main(int argc, char** argv) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}
	std::string i_filename = argv[1];
	std::string o_filename = argv[2];

	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}

	std::map<uint8_t, int> occurrencies;
	while (is) {
		uint8_t val = is.get();
		++occurrencies[val];
	}

	std::ofstream os(o_filename/*, std::ios::binary*/);
	if (!os) {
		return EXIT_FAILURE;
	}
	for (const auto& it : occurrencies) {
		// os << std::hex << std::setw(2) << std::setfill('0') << int(it.first);
		// os << std::dec << "\t" << it.second << std::endl;
		std::println(os, "{:02x}\t{}", it.first, it.second);
	}

	return EXIT_SUCCESS;
}