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

	void quantize(const int& Q) {
		for (auto& x : data_) {
			x = static_cast<int16_t>(round(x / Q));
		}
	}

	void dequantize(const int& Q) {
		for (auto& x : data_) {
			x = static_cast<int16_t>(x * Q);
		}
	}

	const std::vector<int16_t>& data() const { return data_; }
	const int16_t* rawData() const { return data_.data(); }
	const size_t size() const { return data_.size(); }
	const size_t rawSize() const { return data_.size() * sizeof(int16_t); }
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

void writeRaw(const AudioSignal& as, std::ofstream& os) {
	os.write(reinterpret_cast<const char*>(as.rawData()), as.rawSize());
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
	AudioSignal qt(original);
	qt.quantize(2600);
	std::cout << "Quantized signal entropy: " << entropy(qt.data()) << std::endl;
	qt.dequantize(2600);
	std::ofstream output_qt("output_qt.raw", std::ios::binary);
	if (!output_qt) {
		return 1;
	}
	writeRaw(qt, output_qt);

	return 0;
}