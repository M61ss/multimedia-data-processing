#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>

class Image {
private:
	size_t rows_;
	size_t cols_;
	size_t depth_;
	std::string type_;
	std::vector<uint8_t> data_;

public:
	Image(const size_t& rows, const size_t& cols, const size_t& depth, const std::string& type) 
		: cols_(cols), rows_(rows), depth_(depth), type_(type) {
	}

	const uint8_t* operator()(const size_t& i, const size_t& j) const {
		assert(i >= 0 && i < rows_ && j >= 0 && j < cols_);

		return &data_[i * cols_ + j];
	}
	uint8_t* operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t*>(
			static_cast<const Image*>(this)->operator()(i, j));
	}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	const size_t& depth() const { return depth_; }
	const std::string& type() const { return type_; }
	const std::vector<uint8_t>& data() const { return data_; }

	size_t& rows() { return rows_; }
	size_t& cols() { return cols_; }
	size_t& depth() { return depth_; }
	std::string& type() { return type_; }
	std::vector<uint8_t>& data() { return data_; }
};

class GrayscaleImage : public Image {
public:
	GrayscaleImage(const size_t& width, const size_t& height) 
		: Image(width, height, 1, "GRAYSCALE") {
	}
};

GrayscaleImage loadPAM(std::ifstream& is) {
	std::string magicNumber;
	size_t width, height;
	is >> magicNumber;

	std::string line;
	while (std::getline(is, line)) {
		std::stringstream ss(line);
		std::string token;
		ss >> token;

		if (token == "ENDHDR") {
			break;
		}
		if (token == "WIDTH") {
			ss >> width;
		}
		else if ("HEIGHT") {
			ss >> height;
		}
	}

	GrayscaleImage img(width, height);
	for (size_t i = 0; i < img.rows(); i++) {
		for (size_t j = 0; j < img.cols(); j++) {
			is.read(reinterpret_cast<char*>(img(i, j)), sizeof(uint8_t));
		}
	}

	return img;
}

GrayscaleImage computeDiff(const GrayscaleImage& img) {
	GrayscaleImage diff(img.rows(), img.cols());

	diff(0, 0) = *(img(0, 0));

	return diff;
}

int main(int argc, char** argv) {
	if (argc != 4) {
		return 1;
	}

	std::string command = argv[1];
	std::string i_filename = argv[2];
	std::string o_filename = argv[3];
	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		return 1;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		return 1;
	}

	GrayscaleImage img = loadPAM(is);
	GrayscaleImage diff = computeDiff(img);

	return 0;
}