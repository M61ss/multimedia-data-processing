#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <iterator>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Wrong parameter number. Given " << argc << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}

	std::string i_filename = argv[1];
	std::string o_filename = argv[2];
	if (!i_filename.ends_with(".txt")) {
		std::cout << "Wrong input file type. Expected '.txt'." << std::endl;
		return EXIT_FAILURE;
	}
	if (!o_filename.ends_with(".bin")) {
		std::cout << "Wrong output file type. Expected '.bin'." << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream is(i_filename/*, std::ios::binary*/);
	if (!is) {
		std::cout << "Error opening '" << i_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}
	std::ofstream os(o_filename/*, std::ios::binary*/);
	if (!os) {
		std::cout << "Error opening '" << o_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}

	int16_t pack = 0;
	int8_t free_bits = 16;
	int16_t extracted;
	std::vector<int32_t> v{ std::istream_iterator<int32_t>(is), {} };
	for (const auto& x : v) {
		extracted = x & 0x7ff;
		pack |= (extracted >> (16 - free_bits));
		free_bits = 16 - (11 - (16 - free_bits));
		pack <<= free_bits;
		if (free_bits == 0) {
			os << pack;
			free_bits = 16;
		}
	}

	return EXIT_SUCCESS;
}