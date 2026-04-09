#include <fstream>
#include <iostream>
#include <vector>

class Image {
public:
	Image(const size_t& dimX, const size_t& dimY) : dimX_(dimX), dimY_(dimY) {}

	std::vector<std::vector<uint8_t>> setRow(const size_t& index, const std::vector<uint8_t>& row) {
		if (index >= dimY_) throw std::out_of_range("Index out of image Y range.");
		if (row.size() >= dimX_) throw std::out_of_range("Index out of image X range.");
		matrix_[index] = row;

		return matrix_;
	}

	std::vector<uint8_t> getRow(const size_t& index) {
		if (index >= dimY_) throw std::out_of_range("Index out of image Y range.");
		
		return matrix_[index];
	}

	std::vector<std::vector<uint8_t>> setColumn(const size_t& index, const std::vector<uint8_t>& col) {
		if (index >= dimX_) throw std::out_of_range("Index out of image X range.");
		if (col.size() >= dimY_) throw std::out_of_range("Index out of image Y range.");
		for (uint8_t i = 0; i < col.size(); i++) {
			matrix_[i][index] = col[i];
		}

		return matrix_;
	}

	std::vector<uint8_t> getColumn(const size_t& index) {
		if (index >= dimX_) throw std::out_of_range("Index out of image X range.");

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