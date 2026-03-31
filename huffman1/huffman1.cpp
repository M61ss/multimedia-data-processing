#include <iostream>
#include <map>
#include <fstream>

class BitReader {
public:
	BitReader(std::ifstream& is) : is_(is), buffer_(0), n_(0) {
		is_.read(reinterpret_cast<char*>(&buffer_), sizeof(buffer_));
		n_ = 8;
	}

	size_t readBits(uint8_t nBits) {
		size_t out = 0;
		for (uint8_t i = 0; i < nBits; i++) {
			out = (out << 1) | readBit();
		}

		return out;
	}
private:
	std::ifstream& is_;
	uint8_t n_;
	uint8_t buffer_;

	uint8_t readBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(buffer_));
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
};

class Huffman {
public:
	Huffman(std::ifstream& is, std::ofstream& os) : is_(is), os_(os) {}

	std::ofstream& compress() {
		std::map<uint8_t, size_t> frequencies;
		size_t numSymbols = 0;

		os_ << "HUFFMAN1";
		getFrequencies(frequencies, numSymbols);
		os_.write(reinterpret_cast<const char*>(frequencies.size()), sizeof(uint8_t));

		return os_;
	}

	std::ofstream& decompress() {
		char magicNumber[8];
		uint16_t tabEntries;
		std::map<uint8_t, size_t> symbols;

		is_.read(magicNumber, sizeof(magicNumber));

		uint8_t tmp;
		is_.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
		tabEntries = (tmp == 0) ? 256 : tmp;

		BitReader br(is_);
		for (uint8_t i = 0; i < tabEntries; i++) {
			uint8_t symbol = 0;
			is_.read(reinterpret_cast<char*>(&symbol), sizeof(symbol));
			uint8_t codeLength = static_cast<uint8_t>(br.readBits(5));
			symbols[symbol] = br.readBits(codeLength);
		}
		
		return os_;
	}

private:
	std::ifstream& is_;
	std::ofstream& os_;

	std::ifstream& getFrequencies(std::map<uint8_t, size_t>& frequencies, size_t& numSymbols) {
		while (is_) {
			uint8_t curr;
			is_.read(reinterpret_cast<char*>(&curr), sizeof(uint8_t));
			++frequencies[curr];
			++numSymbols;
		}
		for (auto& frequency : frequencies) {
			frequency.second /= numSymbols;
		}

		return is_;
	}
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