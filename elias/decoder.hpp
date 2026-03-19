#pragma once
#include <fstream>
#include <vector>
#include <bit>

template <typename T>
class Decoder {
	std::ifstream& is_;
	uint8_t buffer_;
	uint8_t n_;

public:
	Decoder(std::ifstream& is) : is_(is), buffer_(0), n_(8) {}
	~Decoder() {}

	T decode() {
		buffer_ = is_.get();
	}
};