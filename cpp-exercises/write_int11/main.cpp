#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <bitset>

class BitWriter
{
public:
	BitWriter(std::ostream& os);
	~BitWriter();
	template <typename T> void operator()(int n, const T& val);
	void flushBuffer(uint8_t val = 0);

private:
	std::ostream& os_;
	uint8_t n_;
	uint8_t buffer_;
	void writeBit(uint8_t currBit);
};

BitWriter::BitWriter(std::ostream& os) : os_(os), n_(0), buffer_(0) {}

BitWriter::~BitWriter()
{
	flushBuffer();
}

void BitWriter::writeBit(uint8_t currBit) {
	buffer_ = (buffer_ << 1) | currBit;
	n_++;
	if (n_ == 8) {
		os_.put(buffer_);
		n_ = 0;
	}
}

template <typename T>
void BitWriter::operator()(int n, const T& val) {
	for (int i = (n - 1); i >= 0; i--) {
		uint8_t currBit = (val >> i) & 1;
		writeBit(currBit);
	}
}

void BitWriter::flushBuffer(uint8_t bit) {
	if (bit > 1) bit = 1;
	while (n_ > 0) writeBit(bit);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Wrong parameter number. Given " << argc << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}

	std::string i_filename = argv[1];
	std::string o_filename = argv[2];
	if (!i_filename.ends_with(".txt")) {
		std::cout << "Wrong input file type. Expected '.txt'." << std::endl;
		return EXIT_FAILURE;
	}
	if (!o_filename.ends_with(".bin")) {
		std::cout << "Wrong output file type. Expected '.bin'." << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream is(i_filename/*, std::ios::binary*/);
	if (!is) {
		std::cout << "Error opening '" << i_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening '" << o_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<int32_t> v{ std::istream_iterator<int32_t>(is), {} };

	BitWriter bw(os);
	for (const auto& x : v) {
		bw(11, x);
	}

	return EXIT_SUCCESS;
}