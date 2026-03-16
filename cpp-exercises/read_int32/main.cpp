#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}

	std::vector<int32_t> v;
	int32_t val;
	while (raw_read(is, val)) {
		v.push_back(val);
	}

	std::ofstream os(argv[2]/*, std::ios::binary*/);
	if (!os) {
		return 1;
	}
	//auto& os = std::cout;
	std::copy(begin(v), end(v), std::ostream_iterator<int32_t>(os, "\n"));

	return 0;
}