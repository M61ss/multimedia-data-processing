#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>

class AudioSignal {
private:
	std::vector<int16_t> data_;

	void loadData(std::ifstream& is) {
		is.seekg(0, std::ios::end);
		size_t size = is.tellg();
		is.seekg(0, std::ios::beg);
		data_.resize(size / sizeof(int16_t));
		is.read(reinterpret_cast<char*>(data_.data()), size);
	}
public:
	AudioSignal(std::ifstream& is) : data_() {
		loadData(is);
	}
	AudioSignal(const std::vector<int16_t>& signal) : data_(signal) {}

	const std::vector<int16_t>& data() const { return data_; }
};

double entropy(const std::vector<int16_t>& v) {
	std::map<int16_t, double> frequencies;
	for (size_t i = 0; i < v.size(); i++) {
		++frequencies[v[i]];
	}

	double entropy = 0;
	for (auto& [sym, freq] : frequencies) {
		freq /= v.size();
		entropy += freq * log2(1 / freq);
	}

	return entropy;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}

	AudioSignal original(is);
	std::cout << "Original signal entropy: " << entropy(original.data()) << std::endl;

	return 0;
}