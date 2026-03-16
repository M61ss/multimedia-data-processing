#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& x, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&x), size);
}

template<typename T>
std::ostream& raw_write(std::ostream& os, const std::vector<T>& x) {
	return os.write(reinterpret_cast<const char*>(x.data()), x.size() * sizeof(T));
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1]/*, std::ios::binary*/);
	if (!is) {
		return 1;
	}

	// Option 1
	//std::vector<int32_t> v;
	//int32_t val;
	//while (is >> val) {
	//	v.push_back(val);
	//}

	// Option 2
	//std::vector<int32_t> v;
	//std::istream_iterator<int32_t> begin(is);
	//std::istream_iterator<int32_t> end;
	//for (auto it = begin; it != end; ++it) {
	//	v.push_back(*it);
	//}

	// Option 3
	//std::vector<int32_t> v;
	//std::istream_iterator<int32_t> start(is);
	//std::istream_iterator<int32_t> end;
	//std::copy(start, end, back_inserter(v));

	// Option 4
	//std::istream_iterator<int32_t> start(is);
	//std::istream_iterator<int32_t> end;
	//std::vector<int32_t> v(start, end);

	// Option 4: one liner - Beware The Most Vexing Syntax
	std::vector<int32_t> v{ std::istream_iterator<int32_t>(is), {} };

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return 1;
	}
	//os.write(reinterpret_cast<const char*>(v.data()), v.size() * sizeof(int32_t));
	raw_write(os, v);
	//for (const auto& x : v) {
	//	raw_write(os, x);
	//	//os.write(reinterpret_cast<const char*>(&x), sizeof(int32_t));
	//	//os.put((x >> 0) & 0xff);
	//	//os.put((x >> 8) & 0xff);
	//	//os.put((x >> 16) & 0xff);
	//	//os.put((x >> 24) & 0xff);
	//}

	return 0;
}