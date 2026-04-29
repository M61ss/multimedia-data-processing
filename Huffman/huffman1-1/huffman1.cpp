#include <fstream>
#include <vector>
#include <algorithm>

class BitReader {
private:
	std::ifstream& is_;
	uint8_t buffer_;
	uint8_t n_;

	uint8_t readBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
public:
	BitReader(std::ifstream& is) : is_(is), buffer_(0), n_(0) {}

	size_t readSequence(const uint8_t& len) {
		size_t result = 0;
		for (uint8_t i = 0; i < len; i++) {
			result = (result << 1) | readBit();
		}
		return result;
	}
};

class HuffmanDecoder {
private:
	std::ifstream& is_;
	std::ofstream& os_;
	BitReader br_;
	size_t tableEntries_;
	std::vector<std::pair<uint8_t, std::pair<uint8_t, size_t>>> huffmanTable_;	// len : (sym, code)
	size_t numSymbols_;
	std::vector<uint8_t> data_;

public:
	HuffmanDecoder(std::ifstream& is, std::ofstream& os)
		: is_(is), os_(os), br_(is), tableEntries_(0), huffmanTable_(), numSymbols_(0), data_() {}

	void decompress() {
		readHeader();
		readTableEntries();
		readHuffmanTable();
		readNumSymbols(); 
		readData();
	}

	void readHeader() {
		char header[8] = "";
		is_.read(header, 8 * sizeof(char));
	}

	void readTableEntries() {
		uint8_t tableEntries;
		is_.read(reinterpret_cast<char*>(&tableEntries), sizeof(uint8_t));
		tableEntries_ = tableEntries == 0 ? 256 : static_cast<size_t>(tableEntries);
	}

	void readHuffmanTable() {
		for (size_t i = 0; i < tableEntries_; i++) {
			uint8_t sym = static_cast<uint8_t>(br_.readSequence(8));
			uint8_t len = static_cast<uint8_t>(br_.readSequence(5));
			size_t code = br_.readSequence(len);
			huffmanTable_.push_back(std::make_pair(len, std::make_pair(sym, code)));
		}
		std::sort(huffmanTable_.begin(), huffmanTable_.end());
	}

	void readNumSymbols() {
		numSymbols_ = br_.readSequence(32);
	}

	void readData() {
		for (size_t i = 0; i < numSymbols_; i++) {
			size_t buffer = 0;
			uint8_t alreadyRead = 0;
			for (const auto& [len, repr] : huffmanTable_) {
				const auto& [sym, code] = repr;
				buffer = br_.readSequence(len - alreadyRead);
				alreadyRead = len - alreadyRead;
				if (buffer == code) {
					data_.push_back(sym);
					break;
				}
			}
		}
	}
};

int main(int argc, char** argv) {
	if (argc != 4) {
		return 1;
	}

	std::string command = argv[1];
	std::string i_filename = argv[2];
	std::string o_filename = argv[3];

	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		return 1;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		return 1;
	}

	if (command == "c") {

	}
	else if (command == "d") {
		HuffmanDecoder hd(is, os);
		hd.decompress();
	}
	else {
		return 1;
	}

	return 0;
}