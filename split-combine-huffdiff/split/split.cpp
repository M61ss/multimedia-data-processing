#include <vector>
#include <fstream>
#include <cassert>
#include <sstream>

class Matrix {
private:
	std::vector <uint8_t> data_;
	size_t rows_;
	size_t cols_;

public:
	Matrix(const size_t& rows, const size_t& cols) 
		: rows_(rows), cols_(cols), data_(rows * cols) {
	}

	virtual const uint8_t* operator()(const size_t& i, const size_t& j) const {
		assert(i >= 0 && i < rows_ && j >= 0 && j < cols_);

		return &data_[i * cols_ + j];
	}
	virtual uint8_t* operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t*>(
			static_cast<const Matrix*>(this)->operator()(i, j));
	}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	const std::vector<uint8_t>& data() const { return data_; }

	std::vector<uint8_t>& data() { return data_; }
};

class Image : public Matrix {
private:
	size_t depth_;
	std::string type_;

public:
	Image(const size_t& width, const size_t& height, const size_t& depth, const std::string& type) 
		: Matrix(width, height), depth_(depth), type_(type) {
		data().resize(width * height * depth);
	}

	const uint8_t* operator()(const size_t& i, const size_t& j) const override {
		assert(i >= 0 && i < rows() && j >= 0 && j < cols());

		return &data()[(i * cols() + j) * depth_];
	}
	uint8_t* operator()(const size_t& i, const size_t& j) override {
		return const_cast<uint8_t*>(
			static_cast<const Image*>(this)->operator()(i, j));
	}

	const size_t& depth() const { return depth_; }
	const std::string& type() const { return type_; }
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

	const uint8_t* operator()(const size_t& i, const size_t& j) const override {
		assert(i >= 0 && i < rows() && j >= 0 && j < cols());
		uint8_t pixel[3];
		for (uint8_t i = 0; i < 3; i++) {
			pixel[i] = data()[(i * cols() + j) * depth() + i];
		}

		return pixel;
	}
	uint8_t* operator()(const size_t& i, const size_t& j) override {
		return const_cast<uint8_t*>(
			static_cast<const Image*>(this)->operator()(i, j));
	}

	std::vector<GrayscaleImage> split() const {
		std::vector<GrayscaleImage> channels;
		for (size_t i = 0; i < this->depth(); i++) {
			channels.push_back(GrayscaleImage(this->rows(), this->cols()));
		}

		for (size_t w = 0; w < this->rows(); w++) {
			for (size_t h = 0; h < this->cols(); h++) {
				for (size_t c = 0; c < this->depth(); c++) {
					*(channels[c](w, h)) = this->operator()(w, h)[c];
				}
			}
		}

		return channels;
	}
};

Image loadPAM(std::ifstream& is) {
	std::string magicNumber;
	size_t width, height, depth;
	std::string type;

	is >> magicNumber;

	std::string line;
	while (std::getline(is, line)) {
		std::stringstream ss(line);
		std::string token;
		ss >> token;
		
		if (token == "ENDHDR") {
			break;
		}
		else if (token == "WIDTH") {
			ss >> width;
		}
		else if (token == "HEIGHT") {
			ss >> height;
		}
		else if (token == "DEPTH") {
			ss >> depth;
		}
		else if (token == "TUPLTYPE") {
			ss >> type;
		}
	}

	Image img(width, height, depth, type);
	for (size_t w = 0; w < img.rows(); w++) {
		for (size_t h = 0; h < img.cols(); h++) {
			is.read(reinterpret_cast<char*>(img(w, h)), depth);
		}
	}

	return img;
}

void writePAM(const Image& img, std::ofstream& os) {
	os << "P7" << std::endl
		<< "WIDTH " << img.rows() << std::endl
		<< "HEIGHT " << img.cols() << std::endl
		<< "MAXVAL 255" << std::endl
		<< "DEPTH " << img.depth() << std::endl
		<< "TUPLTYPE " << img.type() << std::endl
		<< "ENDHDR" << std::endl;

	for (size_t w = 0; w < img.rows(); w++) {
		for (size_t h = 0; h < img.cols(); h++) {
			const uint8_t* pixel = img(w, h);
			for (size_t c = 0; c < img.depth(); c++) {
				os.write(reinterpret_cast<const char*>(&pixel[c]), sizeof(uint8_t));
			}
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::string i_filename = argv[1];
	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		return 1;
	}

	RGBImage img = (RGBImage&)loadPAM(is);
	std::vector<GrayscaleImage> channels = img.split();

	std::string o_filenames[3] = { "filename_R.pam", "filename_G.pam", "filename_B.pam" };
	for (size_t i = 0; i < 3; i++) {
		std::ofstream os(o_filenames[i], std::ios::binary);
		if (!os) {
			return 1;
		}
		writePAM(channels[i], os);
	}

	return 0;
}