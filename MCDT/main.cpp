#include <fstream>
#include <iostream>
#include <vector>
#include <map>

class AudioSignal {
private:
	std::ifstream& is_;
	std::vector<int> data_;

	void loadData() {
		int sample = 0;
		while (is_.read(reinterpret_cast<char*>(&sample), sizeof(int16_t))) {
			data_.push_back(sample);
		}
	}

public:
	AudioSignal(std::ifstream& is) : is_(is), data_() {
		loadData();
	}

	double computeEntropy() {
		std::map<int, double> symbols;
		for (const auto& x : data_) {
			++symbols[x];
		}

		size_t numSymbols = data_.size();
		for (auto& [sym, count] : symbols) {
			count /= numSymbols;
		}
		double entropy = 0;
		for (const auto& [sym, prob] : symbols) {
			entropy += prob * log2(1 / prob);
		}

		return entropy;
	}
};

std::vector<int> mcdt(const std::vector<int>& track, const size_t& Q) {
	return std::vector<int>();
}

std::vector<int> imcdt(const std::vector<int>& qtSignal, const size_t& Q) {
	return std::vector<int>();
}

void saveRaw(const std::vector<int>& signal, std::ofstream& os) {

}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::string i_filename = argv[1];

	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		return 1;
	}

	AudioSignal signal(is);
	std::cout << "Original signal entropy: " << signal.computeEntropy() << std::endl;

	return 0;
}