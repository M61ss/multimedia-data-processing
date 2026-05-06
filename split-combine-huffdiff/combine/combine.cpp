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

	const uint8_t* operator()(const size_t& i, const size_t& j) const {
		assert(i >= 0 && i < rows_ && j >= 0 && j < cols_);

		return &data_[i * cols_ + j];
	}
	uint8_t* operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t*>(static_cast<const Matrix*>(this)->operator()(i, j));
	}

	const size_t& rows() const { return rows_; }
	const size_t& cols() const { return cols_; }
	const std::vector<uint8_t>& data() const { return data_; }

	size_t& rows() { return rows_; }
	size_t& cols() { return cols_; }
	std::vector<uint8_t>& data() { return data_; }
};

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::string filename = argv[1];
	std::vector<std::ifstream> iss = {
		std::ifstream(filename + "_R.pam", std::ios::binary),
		std::ifstream(filename + "_G.pam", std::ios::binary),
		std::ifstream(filename + "_B.pam", std::ios::binary),
	};

	return 0;
}