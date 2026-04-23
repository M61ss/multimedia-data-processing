#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

struct Header {
	size_t width_;
	size_t height_;
	size_t depth_;
	size_t maxval_;
	std::string tupltype_;
};

class GrayscalePamImage {
public:
	GrayscalePamImage(std::ifstream& is) : rows_(-1), cols_(-1), hdr_() {
		readHeader(is);
		rows_ = hdr_.width_;
		cols_ = hdr_.height_;
		readImage(is);
	}

	const std::vector<std::vector<uint8_t>>& flip() {
		for (size_t i = 0; i < cols_ / 2; i++) {
			matrix_[i].swap(matrix_[cols_ - 1 - i]);
		}

		return matrix_;
	}

	std::ofstream& print(std::ofstream& os) {
		os << "P7" << std::endl
			<< "WIDTH " << hdr_.width_ << std::endl
			<< "HEIGHT " << hdr_.height_ << std::endl
			<< "DEPTH " << hdr_.depth_ << std::endl
			<< "MAXVAL " << hdr_.maxval_ << std::endl
			<< "TUPLTYPE " << hdr_.tupltype_ << std::endl
			<< "ENDHDR" << std::endl;
		for (const auto& row : matrix_) {
			std::copy(row.begin(), row.end(), std::ostream_iterator<uint8_t>(os));
		}

		return os;
	}

private:
	void readHeader(std::ifstream& is) {
		std::string token;
		is >> token;
		if (token != "P7") {
			return;
		}
		std::string line;
		while (std::getline(is, line)) { 
			if (line[0] == '#') {
				continue;
			}
			std::stringstream ss(line);
			
			ss >> token;
			if (token == "ENDHDR") {
				break;
			}
			std::string value;
			ss >> value;
			if (token == "WIDTH") {
				hdr_.width_ = static_cast<size_t>(std::stoi(value));
			}
			else if (token == "HEIGHT") {
				hdr_.height_ = static_cast<size_t>(std::stoi(value));
			}
			else if (token == "DEPTH") {
				hdr_.depth_ = static_cast<size_t>(std::stoi(value));
			}
			else if (token == "MAXVAL") {
				hdr_.maxval_ = static_cast<size_t>(std::stoi(value));
			}
			else if (token == "TUPLTYPE") {
				hdr_.tupltype_ = value;
			}
		}
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
	Header hdr_;
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