#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <cassert>
#include <numbers>

class MDCT {
private:
	std::vector<double> cos_;
	std::vector<double> w_;
	double recRatio_;
	const size_t& N_;

public:
	MDCT(const size_t& N) : cos_(N * 2 * N), w_(N * 2), N_(N), recRatio_(2 / static_cast<double>(N)) {
		for (size_t k = 0; k < N; k++) {
			for (size_t n = 0; n < N * 2; n++) {
				cos_[k * N + n] = cos((std::numbers::pi / N) * (n + 0.5 + N / 2) * (k + 0.5));
			}
		}
		for (size_t n = 0; n < N * 2; n++) {
			w_[n] = sin((std::numbers::pi / (2 * N)) * (n + 0.5));
		}
	}

	void apply(std::vector<int>& v) {
		std::cout << "MDCT... ";
		for (size_t i = 0; i < N_; i++) {
			v.insert(v.begin(), 0);
		}
		while (v.size() % N_ != 0) {
			v.push_back(0);
		}
		
		std::vector<int> transformed(v.size());
		for (size_t i = 0; i < v.size() - N_; i += N_) {
			for (size_t k = 0; k < N_; k++) {
				double Xk = 0.0;
				for (size_t n = 0; n < N_ * 2; n++) {
					Xk += v[i + n] * w_[n] * cos_[k * 2 * N_ + n];
				}
				transformed[i + k] = static_cast<int>(round(Xk));
			}
		}
		v = transformed;
		std::cout << "done!" << std::endl;
	}

	void invert(std::vector<int>& v) {
		std::cout << "IMDCT... ";
		std::vector<int> reconstructed(v.size() + N_);
		for (size_t i = 0; i < v.size(); i += N_) {
			for (size_t n = 0; n < N_ * 2; n++) {
				double yn = recRatio_ * w_[n];
				double sum = 0.0;
				for (size_t k = 0; k < N_; k++) {
					sum += v[i + k] * cos_[k * 2 * N_ + n];
				}
				reconstructed[i + n] += static_cast<int>(round(yn * sum));
			}
		}
		reconstructed.resize(v.size());
		std::reverse(reconstructed.begin(), reconstructed.end());
		reconstructed.resize(v.size() - N_);
		std::reverse(reconstructed.begin(), reconstructed.end());
		v = reconstructed;
		std::cout << "done!" << std::endl;
	}
};

std::vector<int> loadData(std::ifstream& is) {
	is.seekg(0, std::ios::end);
	size_t size = is.tellg();
	is.seekg(0, std::ios::beg);
	std::vector<int16_t> data(size / sizeof(int16_t));
	is.read(reinterpret_cast<char*>(data.data()), size);

	return std::vector<int>(data.begin(), data.end());
}

void quantize(std::vector<int>& v, const int& Q) {
	for (auto& x : v) {
		x = static_cast<int>(round(static_cast<double>(x) / Q));
	}
}

void dequantize(std::vector<int>& v, const int& Q) {
	for (auto& x : v) {
		x = static_cast<int>(x * Q);
	}
}

double entropy(const std::vector<int>& v) {
	std::map<int, double> frequencies;
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

std::vector<int> computeError(const std::vector<int>& v1, const std::vector<int>& v2) {
	assert(v1.size() == v2.size());
	std::vector<int> error(v1.size());
	for (size_t i = 0; i < v1.size(); i++) {
		error[i] = v1[i] - v2[i];
	}

	return error;
}

void writeRaw(const std::vector<int>& data, const std::string& filename) {
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		throw 1;
	}

	std::vector<int16_t> buffer(data.begin(), data.end());
	os.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(int16_t));
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}

	std::vector<int> original = loadData(is);
	std::cout << "Original signal entropy: " << entropy(original) << std::endl;
	std::vector<int> qt(original.begin(), original.end());
	quantize(qt, 2600);
	std::cout << "Quantized signal entropy: " << entropy(qt) << std::endl;
	dequantize(qt, 2600);
	writeRaw(qt, "output_qt.raw");
	std::vector<int> error = computeError(original, qt);
	writeRaw(error, "error_qt.raw");

	const size_t N = 1024;
	MDCT mdct(N);
	std::vector<int> transformed(original);
	mdct.apply(transformed);
	std::cout << "MDCT coefficient entropy: " << entropy(transformed) << std::endl;
	quantize(transformed, 10000);
	std::cout << "Quantized MDCT coefficient entropy: " << entropy(transformed) << std::endl;
	dequantize(transformed, 10000);
	mdct.invert(transformed);
	writeRaw(transformed, "output.raw");
	error = computeError(original, transformed);
	writeRaw(error, "error.raw");

	return 0;
}