#pragma once
#include <fstream>
#include <bit>

template <typename T> 
class Encoder {
public:
	Encoder(std::ofstream& os) : os_(os), n_(0), buffer_(0) {}

	~Encoder() {
		flush();
	}

	void operator()(T val) {
		val = (val < 0) ? val * -2 : val * 2 + 1;
		uint8_t bitCount = static_cast<uint8_t>(std::bit_width(static_cast<uint64_t>(val)));
		writePadding(bitCount - 1);
		writeValue(val, bitCount);
	}

private:
	uint8_t buffer_;
	uint8_t n_;
	std::ofstream& os_;

	void writeBit(uint8_t currBit) {
		buffer_ = (buffer_ << 1) | currBit;
		n_++;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		}
	}

	void flush(uint8_t bit = 0) {
		if (bit > 1) bit = 1;
		while (n_ > 0) writeBit(bit);
	}

	void writePadding(uint8_t length) {
		for (int i = 0; i < length; i++) {
			writeBit(0);
		}
	}

	void writeValue(const T& val, uint8_t nBits) {
		for (int i = (nBits - 1); i >= 0; i--) {
			uint8_t currBit = (val >> i) & 1;
			writeBit(currBit);
		}
	}
};