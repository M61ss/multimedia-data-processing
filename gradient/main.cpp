#include <fstream>
#include <iostream>
#include <vector>

class Image {
public:
	Image(const size_t& dimX, const size_t& dimY) : dimX_(dimX), dimY_(dimY) {}

	void setRow(const size_t& index, const uint8_t& val) {
		if (index >= dimY_) throw std::out_of_range("Index out of image range.");
		for (auto& col : matrix_) {
			col[index] = val;
		}
	}

	std::vector<uint8_t> getRow(const size_t& index) {
		if (index >= dimY_) throw std::out_of_range("Index out of image range.");
		std::vector<uint8_t> row;
		for (auto& col : matrix_) {
			row.push_back(col[index]);
		}
		
		return row;
	}

	void setColumn(const size_t& index, const uint8_t& val) {
		if (index >= dimX_) throw std::out_of_range("Index out of image range.");
		std::vector<uint8_t> col(dimX_, val);
		matrix_[index] = col;
	}

	std::vector<uint8_t> getColumn(const size_t& index) {
		if (index >= dimX_) throw std::out_of_range("Index out of image range.");

		return matrix_[index];
	}
private:
	std::vector<std::vector<uint8_t>> matrix_;
	size_t dimX_;
	size_t dimY_;
};

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cout << "Wrong parameter number. Given " << argc - 1 << ", expected 1.";
		return 1;
	}



	return 0;
}