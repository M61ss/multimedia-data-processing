#include <fstream>
#include <vector>
#include <map>

class MCDT {
private:
	std::ifstream& is_;
	std::vector<int> data_;

	void loadData() {
		int sample = 0;
		while (is_.read(reinterpret_cast<char*>(&sample), sizeof(int16_t))) {
			data_.push_back(sample);
		}
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

	void dequantize(const size_t& Q) {

	}

	std::vector<int> computeError() {

	}

	void save(const std::vector<int>& track, std::ofstream& os) {

	}

public:
	MCDT(std::ifstream& is) 
		: is_(is), data_() {
	}

	void quantize(const size_t& Q, std::ofstream& os) {
		loadData();
		double entropy = computeEntropy();
	}

};

int main(int argc, char** argv) {
	if (argc != 3) {
		return 1;
	}

	std::string i_filename = argv[1];
	std::string o_filename = argv[2];

	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		return 1;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		return 1;
	}

	MCDT mcdt(is);
	mcdt.quantize(2600, os);

	return 0;
}