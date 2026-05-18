#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <cassert>
#include <numbers>

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
	AudioSignal(const size_t& size) : data_(size) {}

	const int16_t& operator[](const size_t& i) const {
		return data_[i];
	}
	int16_t& operator[](const size_t& i) {
		return data_[i];
	}

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

	std::vector<int16_t>& data() { return data_; }
};

class MDCT {
private:
	std::vector<double> cos_;
	std::vector<double> w_;
	const size_t& N_;

public:
	MDCT(const size_t& N) : cos_(N * 2 * N), w_(N * 2), N_(N) {
		for (size_t k = 0; k < N; k++) {
			for (size_t n = 0; n < N * 2; n++) {
				cos_[k * N + n] = cos((std::numbers::pi / N) * (n + 0.5 + N / 2) * (k + 0.5));
			}
		}
		for (size_t n = 0; n < N * 2; n++) {
			w_[n] = sin((std::numbers::pi / (2 * N)) * (n + 0.5));
		}
	}

	void apply(std::vector<int16_t>& v) {
		while (v.size() % N_ != 0) {
			v.push_back(0);
		}
		for (size_t i = 0; i < N_; i++) {
			v.push_back(0);
			v.insert(v.begin(), 0);
		}

		const size_t stepSize = N_ / 2;
		const size_t rows = v.size() / stepSize - 1;
		for (size_t i = 0; i < rows; i++) {
			for (size_t k = 0; k < N_; k++) {
				
			}
		}
	}

	void invert(std::vector<int16_t>& v) {

	}
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

AudioSignal computeError(const AudioSignal& as1, const AudioSignal& as2) {
	assert(as1.size() == as2.size());
	AudioSignal error(as1.size());
	for (size_t i = 0; i < as1.size(); i++) {
		error[i] = as1[i] - as2[i];
	}

	return error;
}

void writeRaw(const AudioSignal& as, const std::string& filename) {
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		throw 1;
	}
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
	writeRaw(qt, "output_qt.raw");
	AudioSignal error = computeError(original, qt);
	writeRaw(error, "error_qt.raw");

	const size_t N = 1024;
	MDCT mdct(N);
	AudioSignal transformed(original);
	mdct.apply(transformed.data());

	return 0;
}