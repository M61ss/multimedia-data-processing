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
		
	}

	void invert(std::vector<int16_t>& v) {

	}
};

std::vector<int16_t> loadData(std::ifstream& is) {
	is.seekg(0, std::ios::end);
	size_t size = is.tellg();
	is.seekg(0, std::ios::beg);
	std::vector<int16_t> data(size / sizeof(int16_t));
	is.read(reinterpret_cast<char*>(data.data()), size);

	return data;
}

void quantize(std::vector<int16_t>& v, const int& Q) {
	for (auto& x : v) {
		x = static_cast<int16_t>(round(x / Q));
	}
}

void dequantize(std::vector<int16_t>& v, const int& Q) {
	for (auto& x : v) {
		x = static_cast<int16_t>(x * Q);
	}
}

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

std::vector<int16_t> computeError(const std::vector<int16_t>& v1, const std::vector<int16_t>& v2) {
	assert(v1.size() == v2.size());
	std::vector<int16_t> error(v1.size());
	for (size_t i = 0; i < v1.size(); i++) {
		error[i] = v1[i] - v2[i];
	}

	return error;
}

void writeRaw(const std::vector<int16_t>& data, const std::string& filename) {
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		throw 1;
	}
	os.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(int16_t));
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}

	std::vector<int16_t> original = loadData(is);
	std::cout << "Original signal entropy: " << entropy(original) << std::endl;
	std::vector<int16_t> qt(original);
	quantize(qt, 2600);
	std::cout << "Quantized signal entropy: " << entropy(qt) << std::endl;
	dequantize(qt, 2600);
	writeRaw(qt, "output_qt.raw");
	std::vector<int16_t> error = computeError(original, qt);
	writeRaw(error, "error_qt.raw");

	const size_t N = 1024;
	MDCT mdct(N);
	std::vector<int16_t> transformed(original);
	mdct.apply(transformed);

	return 0;
}