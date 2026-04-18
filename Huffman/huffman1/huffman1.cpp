#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>


class BitReader {
public:
	BitReader(std::ifstream& is, std::ofstream& os) : is_(is), os_(os), buffer_(0), n_(0) {}

	size_t readSequence(size_t& output, const size_t& len) {
		for (size_t i = 0; i < len; i++) {
			output = (output << 1) | readBit();
		}

		return output;
	}
private:
	uint8_t readBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
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

	void writeSequence(const size_t& input, const size_t& len) {
		
	}
private:
	std::ifstream& is_;
	std::ofstream& os_;
	uint8_t buffer_;
	uint8_t n_;
};


struct SymbolRepresentation {
	uint8_t len;
	uint16_t code;
};


class HuffmanEncoder {
public:
	HuffmanEncoder(std::ifstream& is, std::ofstream& os) : is_(is), os_(os), tableEntries_(0), numSymbols_(0) {}

	void compress() {
		getFileLength();
		std::map<uint8_t, size_t> frequencies = computeFrequencies();
		createHuffmanTable(frequencies);
	}
private:
	void getFileLength() {
		is_.seekg(0, std::ios::end);
		numSymbols_ = is_.tellg();
		is_.seekg(0, std::ios::beg);
	}

	std::map<uint8_t, size_t> computeFrequencies() {
		std::map<uint8_t, size_t> frequencies;
		uint8_t item = 0;
		while (is_.read(reinterpret_cast<char*>(item), sizeof(uint8_t))) {
			++frequencies[item];
		}
		for (auto& [k, v] : frequencies) {
			v /= numSymbols_;
		}

		return frequencies;
	}

	void createHuffmanTable(const std::map<uint8_t, size_t>& frequencies) {

	}

	void write() {
		os_ << "HUFFMAN1";
		uint8_t tableEntries = (tableEntries_ == 256) ? 0 : static_cast<uint8_t>(tableEntries_);
		os_.write(reinterpret_cast<const char*>(tableEntries), sizeof(uint8_t));
	}

	std::ifstream& is_;
	std::ofstream& os_;
	uint16_t tableEntries_;
	uint32_t numSymbols_;
};


class HuffmanDecoder {
public:
	HuffmanDecoder(std::ifstream& is, std::ofstream& os) : 
		is_(is), os_(os), br(is, os), magicNumber_(""), tableEntries_(0), huffmanTable_(), numSymbols_(0), data_() {}

	void decompress() {
		readMagicNumber();
		readTableEntries();
		readTable();
		readNumSymbols();
		readData();
		writeData();
	}
private:
	void readMagicNumber() {
		is_.read(magicNumber_, 8 * sizeof(char));
	}

	void readTableEntries() {
		uint8_t buffer = 0;
		is_.read(reinterpret_cast<char*>(&buffer), sizeof(uint8_t));
		tableEntries_ = (buffer == 0) ? 256 : buffer;
	}

	void readTable() {
		for (size_t i = 0; i < tableEntries_; i++) {
			size_t sym = 0, len = 0, code = 0;
			br.readSequence(sym, 8);
			br.readSequence(len, 5);
			br.readSequence(code, len);
			huffmanTable_.push_back(std::pair<uint8_t, SymbolRepresentation>(static_cast<uint8_t>(sym), { static_cast<uint8_t>(len), static_cast<uint16_t>(code) }));
		}
		std::sort(huffmanTable_.begin(), huffmanTable_.end(),
			[](std::pair<uint8_t, SymbolRepresentation>& a, std::pair<uint8_t, SymbolRepresentation>& b) {
				return a.second.len < b.second.len;
			});
	}

	void readNumSymbols() {
		size_t buffer = 0;
		br.readSequence(buffer, 32);
		numSymbols_ = static_cast<uint32_t>(buffer);
	}

	void readData() {
		for (uint32_t i = 0; i < numSymbols_; i++) {
			size_t buffer = 0;
			uint16_t bitAlreayRead = 0;
			for (const auto& [sym, repr] : huffmanTable_) {
				uint16_t bitToRead = repr.len - bitAlreayRead;
				br.readSequence(buffer, bitToRead);
				if (buffer == repr.code) {
					data_.push_back(sym);
					break;
				}
				bitAlreayRead += bitToRead;
			}
		}
	}

	void writeData() {
		for (const auto& x : data_) {
			os_.write(reinterpret_cast<const char*>(&x), sizeof(uint8_t));
		}
	}

	std::ifstream& is_;
	std::ofstream& os_;
	BitReader br;
	char magicNumber_[8];
	uint16_t tableEntries_;
	std::vector<std::pair<uint8_t, SymbolRepresentation>> huffmanTable_;
	uint32_t numSymbols_;
	std::vector<uint8_t> data_;
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