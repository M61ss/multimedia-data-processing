#include <iostream>
#include <fstream>
#include <vector>


class HuffmanEncoder {
public:
	HuffmanEncoder(std::ifstream& is, std::ofstream& os) : is_(is), os_(os) {}

	void compress() {

	}
private:
	std::ifstream& is_;
	std::ofstream& os_;
};


class HuffmanDecoder {
public:
	HuffmanDecoder(std::ifstream& is, std::ofstream& os) : is_(is), os_(os) {}

	void decompress() {

	}
private:
	std::ifstream& is_;
	std::ofstream& os_;
};


int main(int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Wrong paramter number. Given " << argc - 1 << ", expected 3." << std::endl;
		return 1;
	}

	std::string mode = argv[1];
	std::string i_filename = argv[2];
	std::string o_filename = argv[3];

	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		std::cout << "Error opening input file '" << i_filename << "'." << std::endl;
		return 1;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening output file '" << o_filename << "'." << std::endl;
		return 1;
	}

	if (mode == "c") {
		HuffmanEncoder he(is, os);
		he.compress();
	}
	else if (mode == "d") {
		HuffmanDecoder hd(is, os);
		hd.decompress();
	}
	else {
		std::cout << "Unsupported operation mode. Given '" << mode << "', expected c (compress) or d (decompress)." << std::endl;
		return 1;
	}

	return 0;
}