#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Wrong paramter number. Given " << argc - 1 << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}
	std::string flag = argv[1];
	std::string i_filename = argv[2];
	std::string o_filename = argv[3];
	if (flag != "c" && flag != "d") {
		std::cout << "First parameter must be 'c' (compression) or 'd' (decompression)." << std::endl;
		return EXIT_FAILURE;
	}
	std::ifstream is(i_filename/*, std::ios::binary*/);
	if (!is) {
		std::cout << "Error opening input file '" << i_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening output file '" << o_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}