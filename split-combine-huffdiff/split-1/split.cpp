#include <vector>
#include <fstream>
#include <cassert>

class Matrix {
private:
	size_t rows_;
	size_t cols_;
	std::vector<uint8_t> data_;

public:
	Matrix(const size_t& rows, const size_t& cols) 
		: cols_(cols), rows_(rows), data_(rows * cols) {
	}

	const uint8_t& operator()(const size_t& i, const size_t& j) const {
		assert(i >= 0 && i < rows_ && j >= 0 && j < cols_);

		return data_[i * cols_ + j];
	}
	uint8_t& operator()(const size_t& i, const size_t& j) {
		return const_cast<uint8_t&>(
			static_cast<const Matrix*>(this)->operator()(i, j)
			);
	}
};