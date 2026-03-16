#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>

typedef struct int11_t
{
	signed int i : 11;
} int11_t;

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

	std::vector<int11_t> v;

	return EXIT_SUCCESS;
}