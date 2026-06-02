#include <fstream>
#include <vector>
#include <map>
#include <bit>

struct BMPHeader {
	uint16_t id_ = 0;
	size_t size_ = 0;
	uint16_t reserved1_ = 0;
	uint16_t reserved2_ = 0;
	size_t offset_ = 0;
};

struct BMPInfoHeader {
	size_t hdrSize_ = 0;
	int width_ = 0;
	int height_ = 0;
	uint16_t nPlanes_ = 0;
	uint16_t bpp_ = 0;
	size_t compression_ = 0;
	size_t rawImgSize_ = 0;
	int hRes_ = 0;
	int vRes_ = 0;
	size_t numColors_ = 0;
	size_t nImportantColors_ = 0;
};

struct Color {
	uint8_t B_ = 0;
	uint8_t G_ = 0;
	uint8_t R_ = 0;
};

class Image {
private:
	size_t rows_;
	size_t cols_;
	std::vector<uint8_t> data_;

public:
	Image() : rows_(-1), cols_(-1) {}
	Image(const size_t& rows, const size_t& cols) : rows_(rows), cols_(cols) {}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	const std::vector<uint8_t>& data() const { return data_; }

	std::vector<uint8_t>& data() { return data_; }
};

Image readBMP(std::istream& is) {
	BMPHeader bmpHdr;
	is.read(reinterpret_cast<char*>(&bmpHdr.id_), 2);
	if (bmpHdr.id_ != 0x4D42) {
		return Image();
	}
	is.read(reinterpret_cast<char*>(&bmpHdr.size_), 4);
	is.read(reinterpret_cast<char*>(&bmpHdr.reserved1_), 2);
	is.read(reinterpret_cast<char*>(&bmpHdr.reserved2_), 2);
	is.read(reinterpret_cast<char*>(&bmpHdr.offset_), 4);

	BMPInfoHeader bmpInfoHdr;
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.hdrSize_), 4);
	if (bmpInfoHdr.hdrSize_ != 40) {
		return Image();
	}
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.width_), 4);
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.height_), 4);
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.nPlanes_), 2);
	if (bmpInfoHdr.nPlanes_ != 1) {
		return Image();
	}
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.bpp_), 2);
	if (bmpInfoHdr.bpp_ != 24) {
		return Image();
	}
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.compression_), 4);
	if (bmpInfoHdr.compression_ != 0) {
		return Image();
	}
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.rawImgSize_), 4);
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.hRes_), 4);
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.vRes_), 4);
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.numColors_), 4);
	is.read(reinterpret_cast<char*>(&bmpInfoHdr.nImportantColors_), 4);

	Image img(bmpInfoHdr.height_, bmpInfoHdr.width_);

	is.seekg(bmpHdr.offset_);
	for (size_t i = 0; i < img.rows(); i++) {
		size_t byteToRead = img.cols() * 3;
		std::vector<uint8_t> buffer(byteToRead);
		for (size_t j = 0; j < img.cols(); j++) {
			is.read(reinterpret_cast<char*>(&buffer[j * 3 + 2]), 1);
			is.read(reinterpret_cast<char*>(&buffer[j * 3 + 1]), 1);
			is.read(reinterpret_cast<char*>(&buffer[j * 3]), 1);
		}
		img.data().insert(img.data().begin(), buffer.begin(), buffer.end());

		size_t bitRead = byteToRead * 8;
		while (bitRead % 32 != 0) {
			is.get();
			bitRead += 8;
		}
	}

	return img;
}

void savePAM(std::ostream& os, const Image& img) {
	os << "P7\n"
		<< "WIDTH " << img.cols() << "\n"
		<< "HEIGHT " << img.rows() << "\n"
		<< "DEPTH 3\n"
		<< "MAXVAL 255\n"
		<< "TUPLTYPE RGB\n"
		<< "ENDHDR\n";
	os.write(reinterpret_cast<const char*>(img.data().data()), img.data().size());
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

	Image img = readBMP(is);
	savePAM(os, img);

	return 0;
}