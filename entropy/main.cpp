#include <cmath>
#include <fstream>
#include <vector>
#include <iterator>
#include <map>
#include <iostream>

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}

	is.seekg(0, std::ios::end);
	size_t nBytes = is.tellg();
	is.seekg(0, std::ios::beg);

	std::vector<uint8_t> data{ std::istream_iterator<uint8_t>(is), {} };

	std::map<uint8_t, double> probs;
	for (const auto& x : data) {
		++probs[x];
	}
	for (auto& [val, prob] : probs) {
		prob /= nBytes;
	}
	
	double entropy = 0;
	for (const auto& [val, prob] : probs) {
		entropy += prob * log2(1 / prob);
	}

	std::cout << "File '" << argv[1] << "':\n\n"
		<< " - Size: " << nBytes << " B\n"
		<< " - Number of symbols: " << probs.size() << "\n"
		<< " - Entropy: " << entropy << "\n";

	return 0;
}