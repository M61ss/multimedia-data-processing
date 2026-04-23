#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <bit>
#include <unordered_map>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& x, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&x), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& x, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&x), size);
}

class bitwriter {
	uint8_t buffer_;
	size_t n_;
	std::ostream& os_;

	void writebit(uint8_t curbit) {
		buffer_ = (buffer_ << 1) | curbit;
		++n_;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		}
	}
public:
	bitwriter(std::ostream& os) : buffer_(0), n_(0), os_(os) {}
	~bitwriter() {
		flush();
	}
	void operator()(uint64_t x, int n) {
		for (int i = n - 1; i >= 0; --i) {
			uint8_t curbit = (x >> i) & 1;
			writebit(curbit);
		}
	}
	void flush(uint8_t bit = 0) {
		while (n_ > 0) {
			writebit(bit);
		}
	}
};

class bitreader {
	uint8_t buffer_;
	size_t n_;
	std::istream& is_;

	uint8_t readbit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
public:
	bitreader(std::istream& is) : buffer_(0), n_(0), is_(is) {}
	std::istream& operator()(uint64_t& x, int n) {
		x = 0;
		for (int i = 0; i < n; ++i) {
			x = (x << 1) | readbit();
		}
		return is_;
	}
};

template<typename T>
struct frequencies {
	std::unordered_map<T, uint32_t> count_;

	void operator()(const T& val) {
		++count_[val];
	}
};

struct node {
	uint8_t sym_ = 0;
	uint32_t count_;
	node* left_ = nullptr;
	node* right_ = nullptr;

	node(uint8_t sym, uint32_t count) :
		sym_(sym), count_(count) {
	}

	node(node* left, node* right) :
		count_(left->count_ + right->count_),
		left_(left), right_(right) {
	}
};


void create_table(std::unordered_map<uint8_t, std::pair<uint32_t, uint32_t>>& table,
	node* n, uint32_t code, uint32_t len)
{
	if (n->left_ == nullptr) {
		table[n->sym_] = std::make_pair(code, len);
	}
	else {
		create_table(table, n->left_, code << 1, len + 1);
		create_table(table, n->right_, (code << 1) + 1, len + 1);
	}
}

int compress(const std::string& infile, const std::string& outfile)
{
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		return 1;
	}

	is.seekg(0, std::ios::end);
	auto pos = is.tellg();
	is.seekg(0, std::ios::beg);
	std::vector<uint8_t> v(pos);
	raw_read(is, v[0], pos);

	// 1) frequencies
	frequencies<uint8_t> f;
	for (const auto& x : v) {
		f(x);
	}

	// 2) create the Huffman tree
	std::vector<node*> nodes;
	for (const auto& x : f.count_) {
		const auto& k = x.first;
		const auto& v = x.second;
		node* n = new node(k, v);
		nodes.push_back(n);
	}

	std::sort(begin(nodes), end(nodes),
		[](node* a, node* b) {
			return a->count_ > b->count_;
		}
	);

	while (nodes.size() > 1) {
		node* n1 = nodes.back();
		nodes.pop_back();
		node* n2 = nodes.back();
		nodes.pop_back();

		node* n = new node(n1, n2);

		auto it = nodes.begin();
		for (size_t i = 0; i < nodes.size(); ++i) {
			if (nodes[i]->count_ <= n->count_) {
				break;
			}
			++it;
		}
		nodes.insert(it, n);
	}
	node* root = nodes.back();
	nodes.pop_back();

	// 3) create the Huffman codes
	std::unordered_map<uint8_t, std::pair<uint32_t, uint32_t>> table;
	create_table(table, root, 0, 0);

	// 4) encode the input stream
	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		return 1;
	}
	os << "HUFFMAN1";
	bitwriter bw(os);
	bw(table.size(), 8);
	for (const auto& x : table) {
		bw(x.first, 8);
		bw(x.second.second, 5);
		bw(x.second.first, x.second.second);
	}
	bw(v.size(), 32);
	for (const auto& x : v) {
		const auto& e = table[x];
		bw(e.first, e.second);
	}
	return 0;
}

int decompress(const std::string& infile, const std::string& outfile)
{
	return 0;
}

int main(int argc, char* argv[])
{
	using namespace std::literals;

	if (argc != 4) {
		return 1;
	}

	if (argv[1] == "c"s) {
		return compress(argv[2], argv[3]);
	}
	else if (argv[1] == "d"s) {
		return decompress(argv[2], argv[3]);
	}
	return 1;
}