#include <fstream>
#include <vector>
#include <bit>

template <typename T>
bool rawRead(T* val, std::istream& is, const size_t& length) {
	return is.read(reinterpret_cast<char*>(val), length).good();
}

template <typename T>
bool rawWrite(const T* val, std::ostream& os, const size_t& length) {
	return os.write(reinterpret_cast<const char*>(val), length).good();
}

class BitReader {
private:
	std::istream& is_;
	uint8_t buffer_ = 0;
	size_t n_ = 0;

	uint8_t readBit() {
		if (n_ == 0) {
			rawRead(&buffer_, is_, 1);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	BitReader(std::istream& is) : is_(is) {}

	size_t readSequence(const size_t& length) {
		size_t out = 0;
		for (size_t i = 0; i < length; i++) {
			out = (out << 1) | readBit();
		}
		return out;
	}
};

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
	const size_t size() const { return data_.size(); }
	const uint8_t* rawData() const { return data_.data(); }

	std::vector<uint8_t>& data() { return data_; }
	uint8_t* rawData() { return data_.data(); }
};

struct G4Header {
	std::string magicNumber_;
	int16_t width_ = 0;
	int16_t height_ = 0;

	G4Header() : magicNumber_(2, 0) {}
};

std::vector<uint8_t> codes = { 0b0001, 0b001, 0b1,0b011,0b000011,0b0000011,0b010,0b000010,0b0000010,0b0000001 };


Image readG4(std::istream& is) {
	G4Header hdr;
	rawRead(hdr.magicNumber_.data(), is, 2);
	if (hdr.magicNumber_ != "G4") {
		return Image();
	}
	rawRead(&hdr.width_, is, 2);
	hdr.width_ = std::byteswap(hdr.width_);
	rawRead(&hdr.height_, is, 2);
	hdr.height_ = std::byteswap(hdr.height_);

	Image img(hdr.height_, hdr.width_);
	BitReader br(is);
	std::vector<uint8_t> referenceLine(img.cols());
	for (size_t i = 0; i < img.rows(); i++) {
		for (size_t j = 0; j < img.cols(); j++) {

		}
		//size_t col = 0;
		//uint8_t a0 = br.readSequence(1);
		//uint8_t b1 = br.readSequence(1);
		//while (b1 == a0 && col <= img.cols()) {
		//	b1 = br.readSequence(1);
		//	++col;
		//}
	}

	return img;
}

void savePAM(std::ostream& os, const Image& img) {
	os << "P7\n"
		<< "WIDTH " << img.cols() << "\n"
		<< "HEIGHT " << img.rows() << "\n"
		<< "DEPTH 1\n"
		<< "MAXVAL 255\n"
		<< "TUPLTYPE GRAYSCALE\n"
		<< "ENDHDR\n";
	rawWrite(img.rawData(), os, img.size());
}

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

	Image img = readG4(is);
	if (img.rows() == -1) {
		return 1;
	}
	savePAM(os, img);

	return 0;
}