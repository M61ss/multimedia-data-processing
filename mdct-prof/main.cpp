#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numbers>
#include <print>
#include <string>
#include <ranges>
#include <vector>

std::vector<int16_t> LoadSamples(const std::string& filename)
{
	std::print("Loading {}... ", filename);
	std::vector<int16_t> samples;
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::println("error!");
		return samples;
	}
	is.seekg(0, std::ios::end);
	auto size = is.tellg();
	is.seekg(0, std::ios::beg);
	samples.resize(size / 2);
	is.read(reinterpret_cast<char*>(samples.data()), size);
	std::println("done.");
	return samples;
}

bool SaveSamples(const std::string& filename, const std::vector<int16_t>& samples)
{
	std::print("Saving {}... ", filename);
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		std::println("error!");
		return false;
	}
	os.write(reinterpret_cast<const char*>(samples.data()), samples.size() * 2);
	std::println("done.");
	return true;
}

template<typename T, typename countT = int32_t>
struct frequency {
	std::map<T, countT> count_;
	void operator()(const T& val) {
		++count_[val];
	}
	double entropy() const {
		double s = 0, n = 0;
		for (const auto& [k, v] : count_) {
			s += v * log2(v);
			n += v;
		}
		return log2(n) - s / n;
	}
};

template<typename T>
void print_entropy(const std::string& name, const std::vector<T>& v)
{
	auto f = std::ranges::for_each(v, frequency<T>()).fun;
	std::println("Entropy of {}: {}", name, f.entropy());
}

struct MDCT {
	static const int N = 1024;
	std::vector<std::vector<double>> cos_;
	std::vector<double> sin_;

	MDCT() : cos_(2 * N, std::vector<double>(N)), sin_(2 * N) {
		using namespace std::numbers;
		for (int k = 0; k < N; ++k) {
			for (int n = 0; n < 2 * N; ++n) {
				cos_[n][k] = cos(pi / N * (n + 0.5 + N / 2) * (k + 0.5));
			}
		}
		for (int n = 0; n < 2 * N; ++n) {
			sin_[n] = sin(pi / (2 * N) * (n + 0.5));
		}
	}

	void apply_mdct(const std::vector<int>& padded, std::vector<int>& transformed, size_t i) {
		for (int k = 0; k < N; ++k) {
			double Xk = 0.0;
			for (int n = 0; n < 2 * N; ++n) {
				double xn = padded[N * i + n];
				double wn = sin_[n];
				Xk += xn * wn * cos_[n][k];
			}
			transformed[N * i + k] = static_cast<int>(lround(Xk));
		}
	}

	void apply_imdct(const std::vector<int>& transformed, std::vector<int>& rec, size_t i) {
		for (int n = 0; n < 2 * N; ++n) {
			double xn = 0.0;
			for (int k = 0; k < N; ++k) {
				double Xk = transformed[N * i + k];
				xn += Xk * cos_[n][k];
			}
			double wn = sin_[n];
			rec[n] = static_cast<int>(lround(2.0 / N * xn * wn));
		}
	}

	std::vector<int> mdct(const std::vector<int16_t>& x) {
		size_t nsamples = x.size();
		size_t nwin = (nsamples + N - 1) / N;
		std::vector<int> padded((nwin + 2) * N);
		std::ranges::copy(x, begin(padded) + N);
		std::vector<int> transformed((nwin + 1) * N);
		for (size_t i = 0; i < nwin + 1; ++i) {
			apply_mdct(padded, transformed, i);
		}
		return transformed;
	}

	std::vector<int16_t> imdct(const std::vector<int>& X) {
		size_t ncoef = X.size();
		size_t nwin = ncoef / N;
		std::vector<int16_t> rec((nwin - 1) * N);
		std::vector<int> rec_prev(2 * N);
		apply_imdct(X, rec_prev, 0);
		std::vector<int> rec_cur(2 * N);
		for (size_t i = 1; i < nwin; ++i) {
			apply_imdct(X, rec_cur, i);
			auto overlapped = std::views::zip(
				rec_prev | std::views::drop(N) | std::views::take(N),
				rec_cur | std::views::take(N))
				| std::views::transform([](const auto& pair) {
				auto [prev, cur] = pair;
				return static_cast<int16_t>(prev + cur);
					});
			std::ranges::copy(overlapped, begin(rec) + (i - 1) * N);
			std::swap(rec_prev, rec_cur);
		}
		return rec;
	}
};

int main(void)
{
	using namespace std::views;
	using namespace std::ranges;

	auto samples = LoadSamples("test.raw");
	if (samples.empty()) {
		return 1;
	}
	print_entropy("samples", samples);

	int Q = 2600;
	auto qsamples = samples
		| std::views::transform(
			[Q](int16_t s) {
				return static_cast<int>(lround(double(s) / Q));
			})
		| to<std::vector<int>>();
	print_entropy("qsamples", qsamples);

	auto rec_qsamples = qsamples
		| std::views::transform(
			[Q](int qs) -> int16_t {
				int rec = qs * Q;
				if (rec < INT16_MIN) return INT16_MIN;
				else if (rec > INT16_MAX) return INT16_MAX;
				else return static_cast<int16_t>(rec);
			})
		| to<std::vector<int16_t>>();
	SaveSamples("output_qt.raw", rec_qsamples);

	auto error_qt = std::views::zip(samples, rec_qsamples)
		| std::views::transform([](const auto& pair) {
		auto [orig, rec] = pair;
		return static_cast<int16_t>(orig - rec);
			})
		| to<std::vector<int16_t>>();
	SaveSamples("error_qt.raw", error_qt);

	MDCT mdct;
	std::print("MDCT... ");
	auto trans = mdct.mdct(samples);
	std::println("done.");
	print_entropy("trans", trans);

	int TQ = 10000;
	auto qtrans = trans
		| std::views::transform(
			[TQ](int s) {
				return static_cast<int>(lround(double(s) / TQ));
			})
		| to<std::vector<int>>();
	print_entropy("qtrans", qtrans);

	auto rec_qtrans = qtrans
		| std::views::transform(
			[TQ](int qc) {
				int rec = qc * TQ;
				if (rec < INT_MIN) return INT_MIN;
				else if (rec > INT_MAX) return INT_MAX;
				else return rec;
			})
		| to<std::vector<int>>();

	std::print("IMDCT... ");
	auto rec_invqtrans = mdct.imdct(rec_qtrans);
	std::println("done.");

	SaveSamples("output.raw", rec_invqtrans);

	auto error = std::views::zip(samples, rec_invqtrans)
		| std::views::transform([](const auto& pair) {
		auto [orig, rec] = pair;
		return static_cast<int16_t>(orig - rec);
			})
		| to<std::vector<int16_t>>();
	SaveSamples("error.raw", error);

	return 0;
}