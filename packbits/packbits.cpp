#include <iostream>
#include <fstream>
#include <vector>

enum State {
	Unknown,
	Run,
	Copy,
	Write
};

class Packbits {
public:
	Packbits(std::ifstream& is, std::ofstream& os) : is_(is), os_(os) {}
	~Packbits() {}

	void compress() {
		State state = Unknown;
		uint8_t readCount = 0;
		uint8_t val;
		std::vector<uint8_t> v;
		while (is_ >> val) {
			if (state == Unknown) {
				state = (v.front() == val) ? Run : Copy;
			} 
			else if (state == Run) {
				state = (v.front() == val) ? Run : Write;
			}
			else {
				state = (v.front() != val) ? Copy : Write;
			}
			if (state == Write) {

				state = Unknown;
			}
			v.push_back(val);
			readCount++;
		}
	}

	void decompress() {

	}

private:
	std::ifstream& is_;
	std::ofstream& os_;
};

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Wrong paramter number. Given " << argc - 1 << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}
	std::string mode		= argv[1];
	std::string i_filename	= argv[2];
	std::string o_filename	= argv[3];
	
	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		std::cout << "Error opening input file '" << i_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening output file '" << o_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}

	Packbits packbits(is, os);
	if (mode == "c") {
		
	}
	else if (mode == "d") {

	}
	else {
		std::cout << "First paramter must be 'c' (compress) or 'd' (decompress)." << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}