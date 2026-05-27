#include <vector>
#include <fstream>

using RGBA = std::array<uint8_t, 4>;

class Image {
private:
	size_t width_;
	size_t height_;
	std::vector<uint8_t> data_;

public:
	Image() : width_(-1), height_(-1), data_() {}
	Image(const size_t& width, const size_t& height) : width_(width), height_(height), data_(width * height) {}

	const RGBA& operator()(const size_t& i, const size_t& j) const {
		return *reinterpret_cast<const RGBA*>(&data_[(i * width_ + j) * 4]);
	}
	RGBA& operator()(const size_t& i, const size_t& j) {
		return const_cast<RGBA&>(static_cast<const Image*>(this)->operator()(i, j));
	}

	const size_t& width() const { return width_; }
	const size_t& height() const { return height_; }
	const std::vector<uint8_t>& data() const { return data_; }

	std::vector<uint8_t>& data() { return data_; }
};

class BitReader {
private:
	std::istream& is_;
	uint8_t buffer_;
	size_t n_;

	size_t readBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
			n_ = 8;
		}
		size_t bit = buffer_ & 1;
		buffer_ >>= 1;
		--n_;
		return bit;
	}
public:
	BitReader(std::istream& is) : is_(is), buffer_(0), n_(0) {}

	size_t readSequence(const size_t& len) {
		size_t val = 0;
		for (size_t i = 0; i < len; i++) {
			val |= readBit() << i;
		}
		return val;
	}
};

void savePAM(std::ostream& os, const Image& img) {
	os << "P7\n"
		<< "WIDTH " << img.width() << "\n"
		<< "HEIGHT " << img.height() << "\n"
		<< "DEPTH 4\n"
		<< "MAXVAL 255\n"
		<< "TUPLTYPE RGBA\n"
		<< "ENDHDR\n";
	os.write(reinterpret_cast<const char*>(img.data().data()), img.width() * img.height() * 4);
}

void eatByte(std::istream& is, const size_t& n) {
	for (size_t i = 0; i < n; i++) {
		is.get();
	}
}

Image loadWebP(std::istream& is) {
	eatByte(is, 4);	// RIFF
	size_t chunkLength = 0;
	is.read(reinterpret_cast<char*>(&chunkLength), 4);
	eatByte(is, 4);	// WEBP
	eatByte(is, 4);	// VP8L
	size_t nByte = 0;
	is.read(reinterpret_cast<char*>(&nByte), 4);
	eatByte(is, 1);	// 0x2F

	BitReader br(is);
	size_t width = br.readSequence(14) + 1;
	size_t height = br.readSequence(14) + 1;
	Image img(width, height);
	size_t transforms = br.readSequence(1);
	size_t colorcache = br.readSequence(1);
	size_t metaprefix = br.readSequence(1);

	return img;
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

	Image img = loadWebP(is);
	savePAM(os, img);

	return 0;
}