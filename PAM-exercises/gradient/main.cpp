#include <fstream>
#include <vector>
#include <fstream>
#include <cassert>

class Matrix {
private:
	std::vector<uint8_t> data_;
	size_t rows_;
	size_t cols_;

public:
	Matrix(const size_t& rows, const size_t& cols)
		: rows_(rows), cols_(cols), data_(rows * cols) {}

	uint8_t& operator()(const size_t& i, const size_t& j) {
		assert(i < rows_ && i >= 0 && j < cols_ && j >= 0);
		return data_[cols_ * i + j];
	}
	const uint8_t& operator()(const size_t& i, const size_t& j) const {
		assert(i < rows_ && i >= 0 && j < cols_ && j >= 0);
		return data_[cols_ * i + j];
	}

	auto rows() const { return rows_; }
	auto cols() const { return cols_; }
	auto size() const { return data_.size(); }
};

void writePAM(std::ofstream& os, const Matrix& img) {
	os << "P7" << std::endl
		<< "WIDTH " << img.rows() << std::endl
		<< "HEIGHT " << img.cols() << std::endl
		<< "DEPTH 1" << std::endl
		<< "MAXVAL 255" << std::endl
		<< "TUPLTYPE GRAYSCALE" << std::endl
		<< "ENDHDR" << std::endl;
	for (size_t i = 0; i < img.rows(); i++) {
		for (size_t j = 0; j < img.cols(); j++) {
			os.write(reinterpret_cast<const char*>(&img(i, j)), sizeof(uint8_t));
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::string o_filename = argv[1];
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		return 1;
	}

	const size_t imageSize = 256;

	Matrix img(imageSize, imageSize);
	for (size_t i = 0; i < img.rows(); i++) {
		for (size_t j = 0; j < img.cols(); j++) {
			img(i, j) = static_cast<uint8_t>(i);
		}
	}

	writePAM(os, img);

	return 0;
}