#pragma once
#include <fstream>
#include <vector>
#include <bit>

template <typename T>
class Decoder {
	std::ifstream& is_;
	uint8_t buffer_;
	uint8_t n_;

	T readBit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	T readValue() {
		T val = 0;
		uint8_t nBits = 0;
		while (val == 0) {
			val = (val << 1) | readBit();
			++nBits;
		}
		for (int i = 0; i < (nBits - 1); i++) {
			val = (val << 1) | readBit();
		}
		return val;
	}

public:
	Decoder(std::ifstream& is) : is_(is), buffer_(0), n_(0) {
		buffer_ = is_.get();
		n_ = 8;
	}
	~Decoder() {}

	std::ifstream& decodeOne(T& val) {
		val = readValue();
		val = (val % 2) == 0 ? val / -2 : (val - 1) / 2;
		return is_;
	}
};