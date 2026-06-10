#include <vector>
#include <fstream>

template <typename T>
bool rawRead(T* val, std::istream& is, const size_t& length) {
	return is.read(reinterpret_cast<char*>(val), length).good();
}

template <typename T>
bool rawWrite(const T* val, std::ostream& os, const size_t& length) {
	return os.write(reinterpret_cast<const char*>(val), length).good();
}

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
	const size_t size() const { return data_.size(); }
	const uint8_t* rawData() const { return data_.data(); }
	const size_t rawSize() const { return data_.size() * 4; }

	std::vector<uint8_t>& data() { return data_; }
	uint8_t* rawData() { return data_.data(); }
};

class BitReader {
private:
	std::istream& is_;
	uint8_t buffer_;
	size_t n_;

	size_t readBitLeastToMost() {
		if (n_ == 0) {
			rawRead(&buffer_, is_, 1);
			n_ = 8;
		}
		size_t bit = buffer_ & 1;
		buffer_ >>= 1;
		--n_;
		return bit;
	}

	size_t readBitMostToLeast() {
		if (n_ == 0) {
			rawRead(&buffer_, is_, 1);
			n_ = 8;
		}
		return (buffer_ >> n_) & 1;
	}
public:
	BitReader(std::istream& is) : is_(is), buffer_(0), n_(0) {}

	size_t readSequenceLeastToMost(const size_t& len) {
		size_t val = 0;
		for (size_t i = 0; i < len; i++) {
			val |= readBitLeastToMost() << i;
		}
		return val;
	}

	size_t readSequenceLeastToMost(const size_t& len) {
		size_t val = 0;
		for (size_t i = 0; i < len; i++) {
			val = (val << 1) | readBitMostToLeast();
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
	rawWrite(img.rawData(), os, img.rawSize());
}

struct WebPHeader {
	std::string magicNumber1_;
	size_t chunkLength_ = 0;
	std::string containerName_;
	std::string magicNumber2_;
	size_t nBytes_ = 0;
	uint8_t signature_ = 0;

	WebPHeader() : magicNumber1_(4, 0), containerName_(4, 0), magicNumber2_(4, 0) {}
};

Image loadWebP(std::istream& is) {
	WebPHeader webpHdr;
	rawRead(webpHdr.magicNumber1_.data(), is, 4);	// RIFF
	if (webpHdr.magicNumber1_ != "RIFF") {
		return Image();
	}
	rawRead(&webpHdr.chunkLength_, is, 4);
	rawRead(webpHdr.containerName_.data(), is, 4);	// WEBP
	if (webpHdr.containerName_ != "WEBP") {
		return Image();
	}
	rawRead(webpHdr.magicNumber2_.data(), is, 4);	// VP8L
	if (webpHdr.magicNumber2_ != "VP8L") {
		return Image();
	}
	rawRead(&webpHdr.nBytes_, is, 4);
	rawRead(&webpHdr.signature_, is, 1);			// 0x2F
	if (webpHdr.signature_ != 0x2F) {
		return Image();
	}

	BitReader br(is);
	size_t width = br.readSequenceLeastToMost(14) + 1;
	size_t height = br.readSequenceLeastToMost(14) + 1;
	Image img(width, height);
	size_t transforms = br.readSequenceLeastToMost(1);
	if (transforms != 0) {
		throw 1;
	}
	size_t colorcache = br.readSequenceLeastToMost(1);
	if (colorcache != 0) {
		throw 1;
	}
	size_t metaprefix = br.readSequenceLeastToMost(1);
	size_t version = br.readSequenceLeastToMost(3);
	if (version != 0) {
		throw 1;
	}

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