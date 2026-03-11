#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <bitset>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Wrong parameter number. Given " << argc << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}

	std::string i_filename = argv[1];
	if (!i_filename.ends_with(".txt")) {
		std::cout << "First file must be .txt." << std::endl;
		return EXIT_FAILURE;
	}
	std::string o_filename = argv[2];
	if (!o_filename.ends_with(".bin")) {
		std::cout << "Second file must be .bin." << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream is(i_filename/*, std::ios::binary */);
	if (!is) {
		std::cout << "Error opening " << i_filename << std::endl;
		return EXIT_FAILURE;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening " << o_filename << std::endl;
		return EXIT_FAILURE;
	}

	int val;
	while (is >> val) {
		std::bitset<32> binary(val);
		binary = binary.flip();
		binary = binary << 1;
		std::string littleEndian = binary.to_string();
		std::reverse(littleEndian.begin(), littleEndian.end());
		if (!(os << littleEndian << std::endl)) {
			std::cout << "Error writing on " << o_filename << "." << std::endl;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}