#include <vector>
#include <fstream>
#include <map>

class Image {
private:
	size_t width_;
	size_t height_;
	std::vector<uint8_t> data_;

public:
	Image() : width_(-1), height_(-1) {}
	Image(const size_t& width, const size_t& height) : width_(width), height_(height), data_(width * height) {}

	const uint8_t& operator()(const size_t& i, const size_t& j) const {
		return data_[i * width_ + j];
	}
	uint8_t& operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t&>(static_cast<const Image*>(this)->operator()(i, j));
	}

	const size_t& width() const { return width_; }
	const size_t& height() const { return height_; }
	const std::vector<uint8_t>& data() const { return data_; }
};

struct TIFFentry {
	int16_t tag_;
	int16_t type_;
	int32_t count_;
	int32_t valueOffset_;
};

Image readTIFF(std::istream& is) {
	std::string byteOrder(2, '\0');
	is.read(byteOrder.data(), 2);
	if (byteOrder != "II") {
		return Image();
	}

	size_t n = 0;
	is.read(reinterpret_cast<char*>(&n), 2);
	if (n != 42) {
		return Image();
	}

	size_t IFDoffset = 0;
	is.read(reinterpret_cast<char*>(&IFDoffset), 4);
	is.seekg(IFDoffset);

	size_t numEntries = 0;
	is.read(reinterpret_cast<char*>(&numEntries), 2);

	std::vector<TIFFentry> entries;
	for (size_t i = 0; i < numEntries; i++) {
		TIFFentry entry;

		is.read(reinterpret_cast<char*>(&entry.tag_), sizeof(entry.tag_));
		is.read(reinterpret_cast<char*>(&entry.type_), sizeof(entry.type_));
		is.read(reinterpret_cast<char*>(&entry.count_), sizeof(entry.count_));
		is.read(reinterpret_cast<char*>(&entry.valueOffset_), sizeof(entry.valueOffset_));
	}
}

void savePAM(std::ostream& os, const Image& img) {
	os << "P7"
		<< "WIDTH " << img.width() << "\n"
		<< "HEIGHT " << img.height() << "\n"
		<< "DEPTH 1\n"
		<< "MAXVAL 255\n"
		<< "TUPLTYPE GRAYSCALE\n"
		<< "ENDHDR\n";
	os.write(reinterpret_cast<const char*>(img.data().data()), sizeof(uint8_t) * img.width() * img.height());
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

	Image img = readTIFF(is);

	return 0;
}