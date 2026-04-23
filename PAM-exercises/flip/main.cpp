#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>

class GrayscalePamImage {
public:
	GrayscalePamImage(std::ifstream& is) : rows_(0), cols_(0), header("") {
		readHeader(is);
		readImage(is);
	}

	const std::vector<std::vector<uint8_t>>& flip() {
		for (size_t i = 0; i < cols_ / 2; i++) {
			matrix_[i].swap(matrix_[cols_ - 1 - i]);
		}

		return matrix_;
	}

	std::ofstream& print(std::ofstream& os) {
		os << header;
		for (const auto& row : matrix_) {
			std::copy(row.begin(), row.end(), std::ostream_iterator<uint8_t>(os));
		}

		return os;
	}

private:
	void readHeader(std::ifstream& is) {
		std::string headerField;
		is >> headerField;
		header.append(headerField + "\n");
		while (true) { 
			is >> headerField;
			header.append(headerField);
			if (headerField == "ENDHDR") break;

			std::string fieldValue;
			is >> fieldValue;
			header.append(" " + fieldValue + "\n");
			if (headerField == "WIDTH") {
				rows_ = static_cast<size_t>(std::stoi(fieldValue));
			}
			else if (headerField == "HEIGHT") {
				cols_ = static_cast<size_t>(std::stoi(fieldValue));
			}
		}
		char newLine;
		is.read(&newLine, sizeof(char));
		header.append("\n");
	}

	void readImage(std::ifstream& is) {
		for (size_t i = 0; i < cols_; i++) {
			matrix_.push_back(std::vector<uint8_t>(rows_));
			is.read(reinterpret_cast<char*>(matrix_[i].data()), sizeof(uint8_t) * rows_);
		}
	}

	size_t rows_;
	size_t cols_;
	std::vector<std::vector<uint8_t>> matrix_;
	std::string header;
};

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Wrong parameter number. Given " << argc - 1 << ", expected 2." << std::endl;
		return 1;
	}

	std::string i_filename = argv[1];
	std::string o_filename = argv[2];

	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		std::cout << "Error opening file '" << i_filename << "'." << std::endl;
		return 1;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening file '" << o_filename << "'." << std::endl;
		return 1;
	}

	GrayscalePamImage grayscalePamImg(is);
	grayscalePamImg.flip();
	grayscalePamImg.print(os);

	return 0;
}