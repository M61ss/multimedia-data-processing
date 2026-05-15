#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <cassert>

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

	const std::vector<int>& data() const { return data_; }
	const size_t& size() const { return data_.size(); }

	std::vector<int>& data() { return data_; }

	double computeEntropy() {
		std::map<int, double> symbols;
		for (const auto& x : data_) {
			++symbols[x];
		}

		size_t numSymbols = data_.size();
		double entropy = 0;
		for (auto& [sym, prob] : symbols) {
			prob /= numSymbols;
			entropy += prob * log2(1 / prob);
		}

		return entropy;
	}

	void quantize(const int& Q) {
		for (auto& x : data_) {
			x /= Q;
		}
	}

	void dequantize(const int& Q) {
		for (auto& x : data_) {
			x *= Q;
		}
	}
};

std::vector<int> computeError(const AudioSignal& as1, const AudioSignal& as2) {
	assert(as1.size() == as2.size());
	std::vector<int> error(as1.size());
	for (size_t i = 0; i < as1.size(); i++) {
		error[i] = as2.data()[i] - as1.data()[i];
	}
	
	return error;
}

std::vector<int> mcdt(const AudioSignal& as) {
	return std::vector<int>();
}

std::vector<int> imcdt(const AudioSignal& as) {
	return std::vector<int>();
}

void saveRaw(const std::vector<int>& data, std::ofstream& os) {
	for (const auto& x : data) {
		os.write(reinterpret_cast<const char*>(&x), sizeof(int16_t));
	}
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

	AudioSignal qtSignal = signal;
	qtSignal.quantize(2600);
	std::cout << "Quantized signal entropy: " << qtSignal.computeEntropy() << std::endl;

	std::ofstream outputQt("output_qt.raw", std::ios::binary);
	if (!outputQt) {
		return 1;
	}
	qtSignal.dequantize(2600);
	saveRaw(qtSignal.data(), outputQt);

	std::ofstream errorQt("error_qt.raw", std::ios::binary);
	if (!errorQt) {
		return 1;
	}
	std::vector<int> qtError = computeError(signal, qtSignal);
	saveRaw(qtError, errorQt);

	return 0;
}