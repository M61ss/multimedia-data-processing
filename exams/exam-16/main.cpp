#include <vector>
#include <fstream>
#include <cstdint>

template <typename T>
bool rawRead(T* val, std::istream& is, const size_t& length) {
	return is.read(reinterpret_cast<char*>(val), length).good();
}

template <typename T>
void rawWrite(T* val, std::ostream& os, const size_t& length) {
	os.write(reinterpret_cast<const char*>(val), length);
}

struct Header {
	std::string magicNumber_;
	uint32_t uncompLength_ = 0;
	uint32_t checkVal_ = 0;

	Header() : magicNumber_(4, 0) {}
};

class LZ4Decoder {
private:
	std::istream& is_;
	Header hdr_;
	std::vector<uint8_t> data_;
	size_t blockCursor_ = 0;
	size_t matchLength_ = 0;

	void readHeader() {
		rawRead(hdr_.magicNumber_.data(), is_, 4);
		rawRead(&hdr_.uncompLength_, is_, 4);
		rawRead(&hdr_.checkVal_, is_, 4);
		if (hdr_.checkVal_ != 0x4D000000) {
			throw "Bad format";
		}
	}

	void readLiterals() {
		uint8_t token = 0;
		rawRead(&token, is_, 1);
		blockCursor_++;
		size_t nLiterals = token >> 4;
		matchLength_ = (token & 0xF) + 4;

		if (uint8_t additional = 0; nLiterals == 15) {
			do {
				rawRead(&additional, is_, 1);
				blockCursor_++;
				nLiterals += additional;
			} while (additional == 255);
		}
		std::vector<uint8_t> buffer(nLiterals);
		rawRead(buffer.data(), is_, nLiterals);
		data_.insert(data_.end(), buffer.begin(), buffer.end());
		blockCursor_ += nLiterals;
	}

	void readMatch() {
		size_t offset = 0;
		rawRead(&offset, is_, 2);
		blockCursor_ += 2;
		if (uint8_t additional = 0; matchLength_ == 19) {
			do {
				rawRead(&additional, is_, 1);
				blockCursor_++;
				matchLength_ += additional;
			} while (additional == 255);
		}
		offset = data_.size() - offset;
		size_t dictLength = data_.size() - offset;
		for (uint32_t i = 0; i < matchLength_; i++) {
			data_.push_back(data_[offset + (i % dictLength)]);
		}
	}

public:
	LZ4Decoder(std::istream& is) : is_(is), hdr_(), data_() {}

	void decode() {
		readHeader();
		uint32_t blockLength = 0;
		while (rawRead(&blockLength, is_, 4)) {
			blockCursor_ = 0;
			readLiterals();
			while (blockCursor_ < blockLength) {
				readMatch();
				readLiterals();
			}
		}
	}

	void print(std::ostream& os) {
		rawWrite(data_.data(), os, data_.size());
	}
};

int main(int argc, char** argv) {
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return 1;
	}

	LZ4Decoder lz4d(is);
	lz4d.decode();
	lz4d.print(os);

	return 0;
}