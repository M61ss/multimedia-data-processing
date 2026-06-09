#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>

template <typename T>
bool rawWrite(const T* val, std::ostream& os, const size_t& length) {
	return os.write(reinterpret_cast<const char*>(val), length).good();
}

template <typename T>
bool rawRead(T* val, std::istream& is, const size_t& length) {
	return is.read(reinterpret_cast<char*>(val), length).good();
}

class Image {
private:
	size_t rows_;
	size_t cols_;
	std::vector<uint8_t> data_;

public:
	Image() : rows_(-1), cols_(-1) {}
	Image(const size_t& rows, const size_t& cols) : rows_(rows), cols_(cols), data_(rows * cols) {}

	const uint8_t& operator()(const size_t& i, const size_t& j) const {
		return data_[i * cols_ + j];
	}
	uint8_t& operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t&>(static_cast<const Image*>(this)->operator()(i, j));
	}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	const std::vector<uint8_t>& data() const { return data_; }
	const uint8_t* rawData() const { return data_.data(); }
	const size_t size() const { return data_.size(); }

	std::vector<uint8_t>& data() { return data_; }
	uint8_t* rawData() { return data_.data(); }
};

struct PGMHeader {
	std::string magicNumber_;
	size_t width_ = -1;
	size_t height_ = -1;
	size_t maxval_ = -1;
};

Image readPGM(std::istream& is) {
	PGMHeader pgmHdr;
	is >> pgmHdr.magicNumber_;
	if (pgmHdr.magicNumber_ != "P5") {
		return Image();
	}
	std::string line;
	while (pgmHdr.maxval_ == -1) {
		std::getline(is, line);
		if (line.starts_with('#')) {
			continue;
		}
		std::stringstream ss(line);
		size_t val = 0;
		ss >> val;
		while (ss) {
			if (pgmHdr.width_ == -1) {
				pgmHdr.width_ = val;
			}
			else if (pgmHdr.height_ == -1) {
				pgmHdr.height_ = val;
			}
			else if (pgmHdr.maxval_ == -1) {
				pgmHdr.maxval_ = val;
				break;
			}
			ss >> val;
		}
	}

	Image img(pgmHdr.height_, pgmHdr.width_);
	rawRead(img.rawData(), is, img.size());

	return img;
}

int compress(std::ostream& os, const Image& img) {
	os << "MULTIRES";
	rawWrite(&img.cols(), os, 4);
	rawWrite(&img.rows(), os, 4);

	Image mlt(img.rows(), img.cols());
	for (size_t i = 0; i < img.rows(); i += 8) {
		for (size_t j = 0; j < img.cols(); j += 8) {
			
		}
	}

	return 0;
}

int decompress(std::istream& is, std::ostream& os) {


	return 0;
}

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
		Image img = readPGM(is);
		compress(os, img);
	}
	else if (mode == "d") {

	}
	else {
		return 1;
	}

	return 0;
}