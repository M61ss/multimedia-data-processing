#include <iostream>
#include <fstream>
#include <vector>


class BitReader {
public:
	BitReader(std::ifstream& is, std::ofstream& os) : is_(is), os_(os), buffer_(0), n_(0) {}

	std::ifstream& readSequence(size_t& output, const size_t& len) {
		for (size_t i = 0; i < len; i++) {
			output = (output << 1) | readBit();
		}
	}
private:
	uint8_t readBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(buffer_), sizeof(uint8_t));
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	std::ifstream& is_;
	std::ofstream& os_;
	uint8_t buffer_;
	uint8_t n_;
};


class BitWriter {
public:
	BitWriter(std::ifstream& is, std::ofstream& os) : is_(is), os_(os), buffer_(0), n_(0) {}

	std::ofstream& writeSequence(const size_t& input, const size_t& len) {
		
	}
private:
	std::ifstream& is_;
	std::ofstream& os_;
	uint8_t buffer_;
	uint8_t n_;
};


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
	HuffmanDecoder(std::ifstream& is, std::ofstream& os) : is_(is), os_(os), nTableItems_(0), magicNumber_("") {}

	std::ifstream& decompress() {
		is_.read(magicNumber_, 8 * sizeof(char));
		is_.read(reinterpret_cast<char*>(nTableItems_), sizeof(uint8_t));

		return is_;
	}
private:
	std::ifstream& readTable() {

	}

	std::ifstream& is_;
	std::ofstream& os_;
	char magicNumber_[8];
	uint8_t nTableItems_;
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