#pragma once
#include <fstream>
#include <vector>
#include <bit>

template <typename T>
class Decoder {
	std::ifstream& is_;
	uint8_t buffer_;
	uint8_t n_;

	void readBit(T& val) {
		--n_;
		uint8_t currBit = (buffer_ >> n_) & 1;
		val = (val << 1) | currBit;
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
	}

	T readValue() {
		T val = 0;
		uint8_t nBits = 0;
		while (val == 0) {
			readBit(val);
			++nBits;
		}
		for (int i = 0; i < (nBits - 1); i++) {
			readBit(val);
		}
		return val;
	}

public:
	Decoder(std::ifstream& is) : is_(is), buffer_(0), n_(0) {
		buffer_ = is_.get();
		n_ = 8;
	}
	~Decoder() {}

	T decodeOne() {
		T val = readValue();
		val = (val % 2) == 0 ? val / -2 : (val - 1) / 2;
		return val;
	}
};