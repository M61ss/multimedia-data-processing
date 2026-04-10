#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

struct RgbPixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

class RgbPamImage {
public:
	RgbPamImage(std::ifstream& is) : pixelX_(0), pixelY_(0), matrix_(0) {
		readHeader(is);
		readImage(is);
	}

	const std::vector<std::vector<RgbPixel>>& mirror() {
		for (auto& row : matrix_) {
			for (size_t i = 0; i < pixelX_ / 2; i++) {
				std::swap(row[i], row[pixelX_ - 1 - i]);
			}
		}

		return matrix_;
	}

	std::ofstream& print(std::ofstream& os) {
		os << header_;
		for (const auto& row : matrix_) {
			for (const auto& pixel : row) {
				os.write(reinterpret_cast<const char*>(&pixel), sizeof(RgbPixel));
			}
			// Why Error?: std::copy(row.begin(), row.end(), std::ostream_iterator<RgbPixel>(os));
		}

		return os;
	}

private:
	std::ifstream& readHeader(std::ifstream& is) {
		std::string headerField;
		is >> headerField;
		header_.append(headerField + "\n");

		while (true) {
			is >> headerField;
			header_.append(headerField);
			if (headerField == "ENDHDR") {
				char newLine;
				is.read(&newLine, sizeof(char));
				header_.append("\n");
				break;
			}

			std::string fieldValue;
			is >> fieldValue;
			header_.append(" " + fieldValue + "\n");
			if (headerField == "WIDTH") {
				pixelX_ = static_cast<size_t>(std::stoi(fieldValue));
			}
			else if (headerField == "HEIGHT") {
				pixelY_ = static_cast<size_t>(std::stoi(fieldValue));
			}
		}

		return is;
	}

	std::ifstream& readImage(std::ifstream& is) {
		for (size_t i = 0; i < pixelY_; i++) {
			matrix_.push_back(std::vector<RgbPixel>(pixelX_));
			is.read(reinterpret_cast<char*>(matrix_[i].data()), sizeof(RgbPixel) * pixelX_);
		}

		return is;
	}

	size_t pixelX_;
	size_t pixelY_;
	std::string header_;
	std::vector<std::vector<RgbPixel>> matrix_;
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

	RgbPamImage rgbPamImage(is);
	rgbPamImage.mirror();
	rgbPamImage.print(os);

	return 0;
}