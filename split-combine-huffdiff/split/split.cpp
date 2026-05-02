#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>

class Matrix {
private:
	std::vector<uint8_t> data_;
	size_t rows_;
	size_t cols_;

public:
	Matrix(const size_t& rows, const size_t& cols) 
		: rows_(rows), cols_(cols), data_(rows * cols) {}

	virtual const uint8_t& operator()(const size_t& i, const size_t& j) const {
		assert(i < rows_ && i >= 0 && j < cols_ && j >= 0);

		return data_[cols_ * i + j];
	}
	virtual uint8_t& operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t&>(
			static_cast<const Matrix*>(this)->operator()(i, j));
	}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	virtual const size_t size() const { return data_.size(); }
	const size_t& rawSize() const { return data_.size(); }
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
		data().resize(width * height * depth);
	}

	const uint8_t& operator()(const size_t& i, const size_t& j) const override {
		assert(i < rows() && i >= 0 && j < cols() && j >= 0);

		return *reinterpret_cast<const uint8_t*>(&data()[(cols() * i + j) * depth_]);
	}
	uint8_t& operator()(const size_t& i, const size_t& j) override {
		return const_cast<uint8_t&>(
			static_cast<const Image*>(this)->operator()(i, j));
	}

	size_t& depth() { return depth_; }
	std::string& type() { return type_; }

	const size_t& depth() const { return depth_; }
	const std::string& type() const { return type_; }
	const size_t size() const override { return data().size() / depth_; }
};

Image loadPAM(std::ifstream& is) {
	std::string magicNumber = "";
	size_t width = -1;
	size_t height = -1;
	size_t depth = -1;
	std::string tupltype = "";
	
	is >> magicNumber;

	std::string line;
	while (std::getline(is, line)) {
		std::stringstream ss(line);
		std::string token;
		ss >> token;

		if (token == "#") {
			continue;
		}
		if (token == "ENDHDR") {
			break;
		}

		if (token == "WIDTH") {
			ss >> width;
		}
		else if (token == "HEIGHT") {
			ss >> height;
		}
		else if (token == "DEPTH") {
			ss >> depth;
		}
		else if (token == "TUPLTYPE") {
			ss >> tupltype;
		}
	}
	Image img(width, height, depth, tupltype);

	for (size_t i = 0; i < width; i++) {
		for (size_t j = 0; j < height; j++) {
			is.read(reinterpret_cast<char*>(&img(i, j)), img.depth());
		}
	}

	return img;
}

void writePAM(const Image& img) {

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

	Image img = loadPAM(is);

	return 0;
}