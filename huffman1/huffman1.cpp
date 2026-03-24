#include <iostream>
#include <vector>
#include <fstream>

class Huffman {
public:
	Huffman(std::ifstream& is, std::ofstream& os) : is_(is), os_(os) {}

	std::ofstream& compress() {
		os_ << "HUFFMAN1";

		return os_;
	}

	std::ofstream& decompress() {

		
		return os_;
	}

private:
	std::ifstream& is_;
	std::ofstream& os_;
};

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Wrong parameter number. Given " << argc - 1 << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}
	std::string mode		= argv[1];
	std::string i_filename	= argv[2];
	std::string o_filename	= argv[3];
	
	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		std::cout << "Error opening input file '" << i_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening output file '" << o_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}

	Huffman huffman(is, os);
	if (mode == "c") {
		huffman.compress();
	}
	else if (mode == "d") {
		huffman.decompress();
	}
	else {
		std::cout << "First parameter must be 'c' (compress) or 'd' (decompress). Given '" << mode << "'." << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}