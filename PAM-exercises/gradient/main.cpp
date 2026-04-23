#include <fstream>
#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>

class Image {
public:
	Image(const size_t& rows, const size_t& cols) : rows_(rows), cols_(cols), matrix_(cols) {}

	std::vector<std::vector<uint8_t>> setRow(const size_t& index, const std::vector<uint8_t>& row) {
		if (index >= cols_) throw std::out_of_range("Index out of Y pixel range.");
		if (row.size() > rows_) throw std::out_of_range("Index out of X pixel range.");
		matrix_[index] = row;

		return matrix_;
	}

	std::vector<uint8_t> getRow(const size_t& index) {
		if (index >= cols_) throw std::out_of_range("Index out of Y pixel range.");
		
		return matrix_[index];
	}

	std::vector<std::vector<uint8_t>> setColumn(const size_t& index, const std::vector<uint8_t>& col) {
		if (index >= rows_) throw std::out_of_range("Index out of X pixel range.");
		if (col.size() > cols_) throw std::out_of_range("Index out of Y pixel range.");
		for (uint8_t i = 0; i < col.size(); i++) {
			matrix_[i][index] = col[i];
		}

		return matrix_;
	}

	std::vector<uint8_t> getColumn(const size_t& index) {
		if (index >= rows_) throw std::out_of_range("Index out of X pixel range.");

		return matrix_[index];
	}

	std::ofstream& writePAM(std::ofstream& os) {
		os << "P7" << std::endl;
		os << "WIDTH " << rows_ << std::endl;
		os << "HEIGHT " << cols_ << std::endl;
		os << "DEPTH 1" << std::endl;
		os << "MAXVAL 255" << std::endl;
		os << "TUPLTYPE GRAYSCALE" << std::endl;
		os << "ENDHDR" << std::endl;
		for (const auto& row : matrix_) {
			std::copy(row.begin(), row.end(), std::ostream_iterator<uint8_t>(os));
		}

		return os;
	}
private:
	std::vector<std::vector<uint8_t>> matrix_;
	size_t rows_;
	size_t cols_;
};

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cout << "Wrong parameter number. Given " << argc - 1 << ", expected 1.";
		return 1;
	}

	std::string o_filename = argv[1];
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening '" << o_filename << "'." << std::endl;
		return 1;
	}

	const size_t imageSize = 256;

	Image graylevelImg(imageSize, imageSize);
	for (size_t i = 0; i < imageSize; i++) {
		std::vector<uint8_t> v(imageSize, static_cast<uint8_t>(i));
		graylevelImg.setRow(i, v);
	}

	graylevelImg.writePAM(os);

	return 0;
}