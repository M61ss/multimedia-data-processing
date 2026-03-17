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
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening '" << o_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}

	int16_t pack = 0;
	int8_t avail_bits = 16;
	const int16_t mask = 0x7ff;
	std::vector<int32_t> v{ std::istream_iterator<int32_t>(is), {} };
	for (const auto& x : v) {
		int8_t written_bits = 11;
		if (avail_bits > 10) {
			pack |= (x & mask);
		}
		else
		{
			int8_t slider = 11 - avail_bits;
			pack |= (x & ((mask >> slider) << slider)) >> slider;
			os << pack;
			pack = 0;
			pack |= (x & (mask >> avail_bits));
			written_bits -= avail_bits;
			avail_bits = 16;
		}
		avail_bits = 16 - (written_bits - (16 - avail_bits));
		pack <<= avail_bits;
	}

	return EXIT_SUCCESS;
}