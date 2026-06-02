#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <bit>
#include <algorithm>

using RGB = std::array<uint8_t, 3>;

class Image {
private:
	size_t rows_;
	size_t cols_;
	std::vector<uint8_t> data_;

public:
	Image() : rows_(-1), cols_(-1) {}
	Image(const size_t& rows, const size_t& cols) : rows_(rows), cols_(cols), data_(rows * cols * 3) {}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	const std::vector<uint8_t>& data() const { return data_; }

	std::vector<uint8_t>& data() { return data_; }
};

std::vector<uint8_t> readRow(std::istream& is, const size_t& len) {
	std::vector<uint8_t> row(len);

	for (size_t j = 0; j < len; j) {
		uint8_t L;
		is.read(reinterpret_cast<char*>(&L), 1);
		if (L <= 127) {
			for (size_t k = 0; k < L; k++) {
				uint8_t val;
				is.read(reinterpret_cast<char*>(&val), 1);
				row[j] = val;
				j++;
			}
		}
		else {
			uint8_t val;
			is.read(reinterpret_cast<char*>(&val), 1);
			for (size_t k = 0; k < L - 128; k++) {
				row[j] = val;
				j++;
			}
		}
	}

	return row;
}

Image readHDR(std::istream& is) {
	std::string line;
	std::getline(is, line);
	if (line != "#?RADIANCE") {
		return Image();
	}

	while (std::getline(is, line)) {
		if (line.starts_with('#')) {
			continue;
		}
		else if (line.empty()) {
			break;
		}
		int sepPos = static_cast<int>(line.find('='));
		if (sepPos == -1) {
			continue;
		}
		std::string var = line.substr(0, sepPos);
		std::string val = line.substr(sepPos + 1, line.size() - 1);
		if (var == "FORMAT" && val != "32-bit_rle_rgbe") {
			return Image();
		}
	}

	std::getline(is, line);
	std::stringstream ss(line);
	size_t width, height;
	std::string token;
	ss >> token;
	if (token != "-Y") {
		return Image();
	}
	ss >> height;
	ss >> token;
	if (token != "+X") {
		return Image();
	}
	ss >> width;

	Image img(height, width);
	std::vector<double> floatImg(img.rows() * img.cols() * 3);
	for (size_t i = 0; i < img.rows(); i++) {
		uint16_t buff;
		is.read(reinterpret_cast<char*>(&buff), 2);
		if (buff != 0x0202) {
			return Image();
		}
		is.read(reinterpret_cast<char*>(&buff), 2);
		uint16_t rowLength = std::byteswap(buff);
		std::vector<uint8_t> R = readRow(is, rowLength);
		std::vector<uint8_t> G = readRow(is, rowLength);
		std::vector<uint8_t> B = readRow(is, rowLength);
		std::vector<uint8_t> E = readRow(is, rowLength);

		for (size_t j = 0; j < img.cols(); j++) {
			size_t idx = (i * img.cols() + j) * 3;
			floatImg[idx] = ((R[j] + 0.5) / 256) * pow(2, E[j] - 128);
			floatImg[idx + 1] = ((G[j] + 0.5) / 256) * pow(2, E[j] - 128);
			floatImg[idx + 2] = ((B[j] + 0.5) / 256) * pow(2, E[j] - 128);
		}
	}

	double min = floatImg[0];
	for (const auto& x : floatImg) {
		if (min > x) {
			min = x;
		}
	}
	double max = floatImg[0];
	for (const auto& x : floatImg) {
		if (max < x) {
			max = x;
		}
	}

	for (size_t i = 0; i < img.rows(); i++) {
		for (size_t j = 0; j < img.cols(); j++) {
			for (size_t k = 0; k < 3; k++) {
				size_t idx = (i * img.cols() + j) * 3 + k;
				img.data()[idx] = static_cast<uint8_t>(255 * pow(((floatImg[idx] - min) / (max - min)), 0.45));
			}
		}
	}

	return img;
}

void writePAM(const Image& img, std::ostream& os) {
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

	Image img = readHDR(is);
	writePAM(img, os);

	return 0;
}