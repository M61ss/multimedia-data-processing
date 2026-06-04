#include <vector>
#include <fstream>
#include <map>
#include <bit>

template <typename T>
void rawRead(std::istream& is, T* var, const size_t& length) {
	is.read(reinterpret_cast<char*>(var), length);
}

template <typename T>
void rawWrite(std::ostream& os, const T* var, const size_t& length) {
	os.write(reinterpret_cast<const char*>(var), length);
}

struct Header {
	std::string magicNumber_;
	size_t outputBytes_ = 0;
	size_t blockSize_ = 0;

	Header() : magicNumber_(4, 0) {}
};

class LZVNDecoder {
private:
	std::istream& is_;
	uint16_t distance_ = 0;
	std::vector<uint8_t> data_;

	Header readHeader() {
		Header hdr;
		rawRead(is_, hdr.magicNumber_.data(), 4);
		if (!is_ || hdr.magicNumber_ != "bvxn") {
			throw 1;
		}
		rawRead(is_, &hdr.outputBytes_, 4);
		rawRead(is_, &hdr.blockSize_, 4);

		return hdr;
	}

	bool validOpcode(const uint8_t& opcode) {
		if (opcode == 0x1E || opcode == 0x26 || opcode == 0x2E || opcode == 0x36 || opcode == 0x3E) {
			return false;
		}
		if ((opcode & 0xF0) == 0x70) {
			return false;
		}
		if ((opcode & 0xF0) == 0xD0) {
			return false;
		}
		return true;
	}

	void readMatch(const size_t& match) {
		size_t startingPoint = data_.size() - distance_;
		for (size_t j = startingPoint; j < startingPoint + match; j++) {
			data_.push_back(data_[j]);
		}
	}

	void readLiterals(const size_t& length) {
		std::vector<uint8_t> buffer(length);
		rawRead(is_, buffer.data(), length);
		data_.insert(data_.end(), buffer.begin(), buffer.end());
	}

public:
	LZVNDecoder(std::istream& is) : is_(is), data_() {}

	std::vector<uint8_t> decode() {
		while (true) {
			Header hdr = readHeader();

			size_t readBytes = 0;
			while (readBytes < hdr.blockSize_) {
				uint8_t opcode = 0;
				rawRead(is_, &opcode, 1);
				++readBytes;
				if (!validOpcode(opcode)) {								// UDEF
					throw 1;
				}
				if (opcode == 0x06) {									// EOS
					return data_;
				}
				else if (opcode == 0x0E || opcode == 0x16) {			// NOP
					continue;
				}
				else if (opcode == 0xE0) {								// LRG_L
					uint16_t length = 0;
					rawRead(is_, &length, 1);
					length += 16;
					readLiterals(length);
					readBytes += length;
				}
				else if (opcode == 0xF0) {								// LRG_M
					uint16_t match = 0;
					rawRead(is_, &match, 1);
					match += 16;
					readMatch(match);
				}
				else if ((opcode & 0xF0) == 0xE0) {						// SML_L
					uint8_t length = opcode & 0xF;
					readLiterals(length);
					readBytes += length;
				}
				else if ((opcode & 0xF0) == 0xF0) {						// SML_M
					uint8_t match = opcode & 0xF;
					readMatch(match);
				}
				else if ((opcode & 0xE0) == 0xA0) {						// MED_D
					uint8_t length = 0;
					length = (opcode & 0x18) >> 3;
					uint8_t buffer1 = 0, buffer2 = 0;
					rawRead(is_, &buffer1, 1);
					rawRead(is_, &buffer2, 1);
					distance_ = buffer2;
					distance_ <<= 6;
					distance_ |= (buffer1 & 0xFC) >> 2;
					uint8_t match = (opcode & 0x07) << 2;
					match |= buffer1 & 0x03;
					match += 3;
					readLiterals(length);
					readMatch(match);
					readBytes += length;
				}
				else if ((opcode & 0x07) == 0x07) {						// LRG_D
					uint8_t length = (opcode & 0xC0) >> 6;
					uint8_t match = (opcode & 0x38) >> 3;
					match += 3;
					distance_ = 0;
					rawRead(is_, &distance_, 2);
					readLiterals(length);
					readMatch(match);
					readBytes += length;
				}
				else if ((opcode & 0x07) == 0x06) {						// PRE_D
					uint8_t length = (opcode & 0xC0) >> 6;
					uint8_t match = (opcode & 0x38) >> 3;
					match += 3;
					readLiterals(length);
					readMatch(match);
					readBytes += length;
				}
				else {													// SML_D
					uint8_t length = (opcode & 0xC0) >> 6;
					uint8_t match = (opcode & 0x38) >> 3;
					match += 3;
					distance_ = opcode & 0x07;
					distance_ <<= 8;
					uint8_t buffer = 0;
					rawRead(is_, &buffer, 1);
					distance_ |= buffer;
					readLiterals(length);
					readMatch(match);
					readBytes += length;
				}
			}
		}

		return data_;
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

	LZVNDecoder lzvnD(is);
	std::vector<uint8_t> data = lzvnD.decode();
	rawWrite(os, data.data(), data.size());

	return 0;
}