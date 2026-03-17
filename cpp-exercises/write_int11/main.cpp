#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>

class BitWriter
{
public:
	BitWriter(std::ostream& os);
	~BitWriter();

private:
	std::ostream& os_;
	uint8_t n_;
	uint8_t buffer_;
};

BitWriter::BitWriter(std::ostream& os) : os_(os), n_(0), buffer_(0) {}

BitWriter::~BitWriter()
{
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Wrong parameter number. Given " << argc << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}

	std::string i_filename = argv[1];
	std::string o_filename = argv[2];
	if (!i_filename.ends_with(".txt")) {
		std::cout << "Wrong input file type. Expected '.txt'." << std::endl;
		return EXIT_FAILURE;
	}
	if (!o_filename.ends_with(".bin")) {
		std::cout << "Wrong output file type. Expected '.bin'." << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream is(i_filename/*, std::ios::binary*/);
	if (!is) {
		std::cout << "Error opening '" << i_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		std::cout << "Error opening '" << o_filename << "'." << std::endl;
		return EXIT_FAILURE;
	}

	

	return EXIT_SUCCESS;
}