#include <vector>
#include <fstream>
#include <iostream>

class BitWriter {
private:
	std::ostream& os_;
	uint8_t buffer_;
	size_t n_;

	void writeBit(const uint8_t& bit) {
		++n_;
		buffer_ = (buffer_ << 1) | bit;
		if (n_ == 8) {
			os_.write(reinterpret_cast<const char*>(&buffer_), 1);
			n_ = 0;
		}
	}

	void flush(const uint8_t& bit = 0) {
		while (n_ > 0) {
			writeBit(bit);
		}
	}

public:
	BitWriter(std::ostream& os) : os_(os), buffer_(0), n_(0) {}
	~BitWriter() {
		flush();
	}

	void writeSequence(const size_t& val, const size_t& len) {
		for (int i = static_cast<int>(len) - 1; i >= 0; i--) {
			uint8_t bit = (val >> i) & 1;
			writeBit(bit);
		}
	}
};

class EliasEncoder {
private:
	std::istream& is_;
	std::ostream& os_;

public:
	EliasEncoder(std::istream& is, std::ostream& os) : os_(os), is_(is) {}
};

int main(int argc, char** argv) {
	if (argc != 4) {
		return 1;
	}

	std::string mode(argv[1]);
	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		return 1;
	}
	std::ofstream os(argv[3], std::ios::binary);
	if (!os) {
		return 1;
	}

	if (mode == "c") {
		EliasEncoder ee(is, os);
	}
	else if (mode == "d") {

	}
	else {
		return 1;
	}

	return 0;
}