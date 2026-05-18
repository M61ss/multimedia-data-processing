#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include <numbers>

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
	const size_t size() const { return data_.size(); }

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
			x = static_cast<int>(round(x / Q));
		}
	}

	void dequantize(const int& Q) {
		for (auto& x : data_) {
			x *= Q;
		}
	}

	void mdct(const size_t& windowSize) {
		for (size_t i = 0; i < windowSize; i++) {
			data_.insert(data_.begin(), 0);
			data_.push_back(0);
		}
		while ((data_.size() % windowSize) != 0) {
			data_.push_back(0);
		}

		std::vector<int> blocks;
		const size_t stepSize = windowSize / 2;
		const size_t rows = data_.size() / stepSize - 1;
		for (size_t i = 0; i < rows; i++) {
			for (size_t j = 0; j < windowSize; j++) {
				blocks.push_back(data_[i * stepSize + j]);
			}
		}
		data_.clear();

		for (size_t i = 0; i < rows; i++) {
			for (size_t k = 0; k < windowSize; k++) {
				int Xk = 0;
				for (size_t n = 0; n < windowSize * 2; n++) {
					const int xn = blocks[i * windowSize * 2 + n];
					const double wn = sin((std::numbers::pi / 2 * windowSize) * (n + 0.5));
					Xk += static_cast<int>(xn * wn * cos((std::numbers::pi / windowSize) * (n + 0.5 + windowSize / 2) * (k + 0.5)));
				}
				data_.push_back(Xk);
			}
		}
	}

	void imdct(const size_t& windowSize) {
		//std::vector<int> antitrasformed;
		//for (size_t i = 0; i < data_.size() / windowSize; i++) {
		//	for (size_t n = 0; n < windowSize * 2; n++) {
		//		const double wn = sin((std::numbers::pi / 2 * windowSize) * (n + 0.5));
		//		int yn = (2 / windowSize) * wn;
		//		int sum = 0;
		//		for (size_t k = 0; k < windowSize; k++) {
		//			const int& Xk = data_[i * windowSize + k];
		//			sum += Xk * cos((std::numbers::pi / 2) * (n + 0.5 + windowSize / 2) * (k + 0.5));
		//		}
		//		yn *= sum;
		//		antitrasformed.push_back(yn);
		//	}
		//}

		//data_ = antitrasformed;
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

	AudioSignal quantized = signal;
	quantized.quantize(2600);
	std::cout << "Quantized signal entropy: " << quantized.computeEntropy() << std::endl;

	std::ofstream outputQt("output_qt.raw", std::ios::binary);
	if (!outputQt) {
		return 1;
	}
	quantized.dequantize(2600);
	saveRaw(quantized.data(), outputQt);

	std::ofstream errorQt("error_qt.raw", std::ios::binary);
	if (!errorQt) {
		return 1;
	}
	std::vector<int> qtError = computeError(signal, quantized);
	saveRaw(qtError, errorQt);

	AudioSignal compressed = signal;
	compressed.mdct(1024);
	compressed.quantize(10000);
	std::cout << "Compressed signal entropy: " << quantized.computeEntropy() << std::endl;
	compressed.dequantize(10000);
	compressed.imdct(1024);
	std::ofstream outputCompressed("output.raw", std::ios::binary);
	if (!outputCompressed) {
		return 1;
	}
	saveRaw(compressed.data(), outputCompressed);

	std::ofstream errorCompression("error.raw", std::ios::binary);
	if (!errorCompression) {
		return 1;
	}
	std::vector<int> compressionError = computeError(signal, compressed);
	saveRaw(compressionError, errorCompression);

	return 0;
}