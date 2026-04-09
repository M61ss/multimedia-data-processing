#include <fstream>
#include <iostream>
#include <vector>

class Image {
public:
	Image(const size_t& dimX, const size_t& dimY) : pixelX_(dimX), pixelY_(dimY) {}

	std::vector<std::vector<uint8_t>> setRow(const size_t& index, const std::vector<uint8_t>& row) {
		if (index >= pixelY_) throw std::out_of_range("Index out of Y pixel range.");
		if (row.size() >= pixelX_) throw std::out_of_range("Index out of X pixel range.");
		matrix_[index] = row;

		return matrix_;
	}

	std::vector<uint8_t> getRow(const size_t& index) {
		if (index >= pixelY_) throw std::out_of_range("Index out of Y pixel range.");
		
		return matrix_[index];
	}

	std::vector<std::vector<uint8_t>> setColumn(const size_t& index, const std::vector<uint8_t>& col) {
		if (index >= pixelX_) throw std::out_of_range("Index out of X pixel range.");
		if (col.size() >= pixelY_) throw std::out_of_range("Index out of Y pixel range.");
		for (uint8_t i = 0; i < col.size(); i++) {
			matrix_[i][index] = col[i];
		}

		return matrix_;
	}

	std::vector<uint8_t> getColumn(const size_t& index) {
		if (index >= pixelX_) throw std::out_of_range("Index out of X pixel range.");

		return matrix_[index];
	}
private:
	std::vector<std::vector<uint8_t>> matrix_;
	size_t pixelX_;
	size_t pixelY_;
};

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cout << "Wrong parameter number. Given " << argc - 1 << ", expected 1.";
		return 1;
	}



	return 0;
}