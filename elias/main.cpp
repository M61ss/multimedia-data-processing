#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include "encoder.hpp"
#include "decoder.hpp"

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Wrong paramter number. Given " << argc - 1 << ", expected 3." << std::endl;
		return EXIT_FAILURE;
	}
	std::string flag = argv[1];
	std::string i_filename = argv[2];
	std::string o_filename = argv[3];
	if (flag == "c") {
		std::ifstream is(i_filename/*, std::ios::binary*/);
		if (!is) {
			std::cout << "Error opening input file '" << i_filename << "'." << std::endl;
			return EXIT_FAILURE;
		}
		std::ofstream os(o_filename, std::ios::binary);
		if (!os) {
			std::cout << "Error opening output file '" << o_filename << "'." << std::endl;
			return EXIT_FAILURE;
		}

		std::vector<int32_t> v{ std::istream_iterator<int32_t>(is), {} };

		Encoder<int32_t> encoder(os);
		for (auto& x : v) {
			encoder(x);
		}
	}
	else if (flag == "d") {
		std::ifstream is(i_filename, std::ios::binary);
		if (!is) {
			std::cout << "Error opening input file '" << i_filename << "'." << std::endl;
			return EXIT_FAILURE;
		}
		std::ofstream os(o_filename/*, std::ios::binary*/);
		if (!os) {
			std::cout << "Error opening output file '" << o_filename << "'." << std::endl;
			return EXIT_FAILURE;
		}

		Decoder<int32_t> decoder(is);
		std::vector<int32_t> v;
		while (is) {
			v.push_back(decoder.decode());
		}

		for (const auto& x : v) {
			os << x << std::endl;
		}
	}
	else {
		std::cout << "First parameter must be 'c' (compression) or 'd' (decompression)." << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}