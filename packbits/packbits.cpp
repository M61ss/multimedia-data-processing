#include <iostream>
#include <fstream>
#include <vector>

enum State {
	Unknown,
	Run,
	Copy
};

class Packbits {
public:
	Packbits(std::ifstream& is, std::ofstream& os) : is_(is), os_(os) {}
	~Packbits() {}

	std::ofstream& compress() {
		State state = Unknown;
		bool write = false;
		uint8_t val;
		std::vector<uint8_t> v;
		while (is_.read(reinterpret_cast<char *>(&val), sizeof(uint8_t))) {
			if (write == true) {
				if (state == Copy) {
					writeCopyPack(v, state);
				}
				else {
					writeRunPack(v, state);
				}
				write = false;
			}

			if (state == Unknown && !v.empty()) {
				state = (v.back() == val) ? Run : Copy;
			} 
			else if (state == Run) {
				write = (v.back() == val) ? false : true;
				if (v.size() == 128) write = true;
			}
			else if (state == Copy) {
				write = (v.back() != val) ? false : true;
				if (v.size() == 129) write = true;
			}
			v.push_back(val);
		}
		writeEnd(v, state);
		
		return os_;
	}

	std::ofstream& decompress() {
		uint8_t command;
		while (is_.read(reinterpret_cast<char *>(&command), sizeof(uint8_t))) {
			if (command == 128) {
				break;
			}
			else if (command < 128) {
				uint8_t tmp;
				for (uint8_t i = 0; i < command + 1; i++) {
					is_.read(reinterpret_cast<char*>(&tmp), sizeof(uint8_t));
					os_.write(reinterpret_cast<const char*>(&tmp), sizeof(uint8_t));
				}
			}
			else {
				uint8_t val;
				is_.read(reinterpret_cast<char*>(&val), sizeof(uint8_t));
				for (uint8_t i = 0; i < 257 - command; i++) {
					os_.write(reinterpret_cast<const char*>(&val), sizeof(uint8_t));
				}
			}
		}

		return os_;
	}

private:
	std::ifstream& is_;
	std::ofstream& os_;

	std::ofstream& writeCopyPack(std::vector<uint8_t>& v, State& state, const uint8_t& excess = 2) {
		uint8_t writeCount = static_cast<uint8_t>(v.size()) - excess;
		uint8_t command = writeCount - 1;
		os_.write(reinterpret_cast<const char*>(&command), sizeof(uint8_t));
		for (uint8_t i = 0; i < writeCount; i++) {
			os_.write(reinterpret_cast<const char*>(&v[0]), sizeof(uint8_t));
			v.erase(v.begin());
		}
		if (v.size() > 1) state = (v.back() == v.front()) ? Run : Copy;

		return os_;
	}

	std::ofstream& writeRunPack(std::vector<uint8_t>& v, State& state, const uint8_t& excess = 1) {
		uint8_t writeCount = static_cast<uint8_t>(v.size()) - excess;
		uint8_t command = 257 - writeCount;
		os_.write(reinterpret_cast<const char*>(&command), sizeof(uint8_t));
		os_.write(reinterpret_cast<const char*>(&v[0]), sizeof(uint8_t));
		v.erase(v.begin(), v.end() - 1);
		state = Unknown;

		return os_;
	}

	std::ofstream& writeEnd(std::vector<uint8_t>& v, State& state) {
		if (!v.empty()) {
			if (state == Copy) {
				writeCopyPack(v, state, 0);
			}
			else {
				writeRunPack(v, state, 0);
			}
		}
		uint8_t eof = 128;
		os_.write(reinterpret_cast<const char*>(&eof), sizeof(uint8_t));

		return os_;
	}
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
		packbits.compress();
	}
	else if (mode == "d") {
		packbits.decompress();
	}
	else {
		std::cout << "First paramter must be 'c' (compress) or 'd' (decompress)." << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}