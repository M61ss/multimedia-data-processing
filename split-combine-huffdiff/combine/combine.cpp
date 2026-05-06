#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>

class Matrix {
private:
	size_t rows_;
	size_t cols_;
	std::vector<uint8_t> data_;

public:
	Matrix(const size_t& rows, const size_t& cols) 
		: rows_(rows), cols_(cols), data_(rows* cols) {
	}

	virtual const uint8_t* operator()(const size_t& i, const size_t& j) const {
		assert(i >= 0 && i < rows_ && j >= 0 && j < cols_);

		return &data_[i * cols_ + j];
	}
	virtual uint8_t* operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t*>(static_cast<const Matrix*>(this)->operator()(i, j));
	}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	const std::vector<uint8_t>& data() const { return data_; }

	size_t& rows() { return rows_; }
	size_t& cols() { return cols_; }
	std::vector<uint8_t>& data() { return data_; }
};

class Image : public Matrix {
private:
	size_t depth_;
	std::string type_;

public:
	Image(const size_t& width, const size_t& height, const size_t& depth, const std::string& type)
		: Matrix(width, height), depth_(depth), type_(type) {
		data().resize(width * depth * height);
	}

	const size_t& depth() const { return depth_; }
	const std::string& type() const { return type_; }

	size_t& depth() { return depth_; }
	std::string& type() { return type_; }
};

class GrayscaleImage : public Image {
public:
	GrayscaleImage(const size_t& width, const size_t& height)
		: Image(width, height, 1, "GRAYSCALE") {
	}
};

class RGBImage : public Image {
public:
	RGBImage(const size_t& width, const size_t& height)
		: Image(width, height, 3, "RGB") {
	}
	RGBImage(const size_t& width, const size_t& height, const std::vector<GrayscaleImage>& channels) 
		: Image(width, height, 3, "RGB") {
		assert(channels.size() == 3);
		assert(width == channels[0].rows() && width == channels[1].rows() && width == channels[2].rows());
		assert(height == channels[0].cols() && height == channels[1].cols() && height == channels[2].cols());

		for (size_t i = 0; i < this->rows(); i++) {
			for (size_t j = 0; j < this->cols(); j++) {
				for (size_t k = 0; k < 3; k++) {
					this->operator()(i,j)[k] = *(channels[k](i, j));
				}
			}
		}
	}

	const uint8_t* operator()(const size_t& i, const size_t& j) const override {
		assert(i >= 0 && i < rows() && j >= 0 && j < cols());

		return &data()[(i * cols() + j) * 3];
	}
	uint8_t* operator()(const size_t& i, const size_t& j) override {
		return const_cast<uint8_t*>(static_cast<const RGBImage*>(this)->operator()(i, j));
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
		else if (token == "HEIGHT") {
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

void writePAM(const Image& img, std::ofstream& os) {
	os << "P7" << std::endl
		<< "WIDTH " << img.rows() << std::endl
		<< "HEIGHT " << img.cols() << std::endl
		<< "DEPTH " << img.depth() << std::endl
		<< "MAXVAL 255" << std::endl
		<< "TUPLTYPE " << img.type() << std::endl
		<< "ENDHDR" << std::endl;

	for (size_t i = 0; i < img.rows(); i++) {
		for (size_t j = 0; j < img.cols(); j++) {
			os.write(reinterpret_cast<const char*>(img(i, j)), img.depth() * sizeof(uint8_t));
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::string filename = argv[1];
	std::vector<std::ifstream> iss;
	iss.push_back(std::ifstream(filename + "_R.pam", std::ios::binary));
	iss.push_back(std::ifstream(filename + "_G.pam", std::ios::binary));
	iss.push_back(std::ifstream(filename + "_B.pam", std::ios::binary));
	

	std::vector<GrayscaleImage> channels;
	for (size_t i = 0; i < 3; i++) {
		if (!iss[i]) {
			return 1;
		}
		channels.push_back(loadPAM(iss[i]));
	}
	size_t width = channels[0].rows();
	size_t height = channels[0].cols();
	RGBImage img(width, height, channels);

	std::ofstream os(filename + "_reconstructed.pam", std::ios::binary);
	writePAM(img, os);

	return 0;
}