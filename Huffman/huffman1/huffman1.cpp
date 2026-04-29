#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

class BitReader {
private:
	std::ifstream& is_;
	uint8_t buffer_;
	uint8_t n_;

	uint8_t readBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
public:
	BitReader(std::ifstream& is) : is_(is), buffer_(0), n_(0) {}

	size_t readSequence(const uint8_t& len) {
		size_t result = 0;
		for (uint8_t i = 0; i < len; i++) {
			result = (result << 1) | readBit();
		}
		return result;
	}
};

class HuffmanDecoder {
private:
	std::ifstream& is_;
	std::ofstream& os_;
	BitReader br_;
	size_t tableEntries_;
	std::vector<std::pair<uint8_t, std::pair<uint8_t, size_t>>> huffmanTable_;	// len : (sym, code)
	size_t numSymbols_;
	std::vector<uint8_t> data_;

public:
	HuffmanDecoder(std::ifstream& is, std::ofstream& os)
		: is_(is), os_(os), br_(is), tableEntries_(0), huffmanTable_(), numSymbols_(0), data_() {}

	void decompress() {
		readHeader();
		readTableEntries();
		readHuffmanTable();
		readNumSymbols(); 
		readData();
		printData();
	}

	void readHeader() {
		char header[8] = "";
		is_.read(header, 8 * sizeof(char));
	}

	void readTableEntries() {
		uint8_t tableEntries;
		is_.read(reinterpret_cast<char*>(&tableEntries), sizeof(uint8_t));
		tableEntries_ = tableEntries == 0 ? 256 : static_cast<size_t>(tableEntries);
	}

	void readHuffmanTable() {
		for (size_t i = 0; i < tableEntries_; i++) {
			uint8_t sym = static_cast<uint8_t>(br_.readSequence(8));
			uint8_t len = static_cast<uint8_t>(br_.readSequence(5));
			size_t code = br_.readSequence(len);
			huffmanTable_.push_back(std::make_pair(len, std::make_pair(sym, code)));
		}
		std::sort(huffmanTable_.begin(), huffmanTable_.end());
	}

	void readNumSymbols() {
		numSymbols_ = br_.readSequence(32);
	}

	void readData() {
		for (size_t i = 0; i < numSymbols_; i++) {
			size_t buffer = 0;
			uint8_t alreadyRead = 0;
			for (const auto& [len, repr] : huffmanTable_) {
				const auto& [sym, code] = repr;
				uint8_t toBeRead = len - alreadyRead;
				buffer = (buffer << toBeRead) | br_.readSequence(toBeRead);
				alreadyRead += toBeRead;
				if (buffer == code) {
					data_.push_back(sym);
					break;
				}
			}
		}
	}

	void printData() {
		for (const auto& x : data_) {
			os_.write(reinterpret_cast<const char*>(&x), sizeof(uint8_t));
		}
	}
};

class BitWriter {
private:
	std::ofstream& os_;
	uint8_t buffer_;
	uint8_t n_;

	void writeBit(const uint8_t& bit) {
		buffer_ = (buffer_ << 1) | bit;
		++n_;
		if (n_ == 8) {
			os_.write(reinterpret_cast<const char*>(&buffer_), sizeof(uint8_t));
			n_ = 0;
		}
	}

	void flush(uint8_t val = 0) {
		while (n_ > 0) {
			writeBit(val);
		}
	}
public:
	BitWriter(std::ofstream& os) 
		: os_(os), buffer_(0), n_(0) {}
	~BitWriter() {
		flush();
	}

	void writeSequence(const size_t& value, const uint8_t& len) {
		for (uint8_t i = 0; i < len; i++) {
			uint8_t bit = static_cast<uint8_t>(value >> (len - (i + 1)));
			writeBit(bit);
		}
	}
};

struct Node {
	uint8_t sym_ = 0;
	size_t count_;
	Node* left_ = nullptr;
	Node* right_ = nullptr;

	Node(Node* left, Node* right) 
		: left_(left), right_(right), count_(left->count_ + right->count_) {}

	Node(const uint8_t& sym, const size_t& count) 
		: sym_(sym), count_(count) {}
};

class HuffmanEncoder {
private:
	std::ifstream& is_;
	std::ofstream& os_;
	BitWriter bw_;
	std::string magicNumber_;
	size_t tableEntries_;
	std::map<uint8_t, std::pair<uint8_t, size_t>> huffmanTable_;	// sym : (len, code)
	size_t numSymbols_;
	std::vector<uint8_t> data_;

public:
	HuffmanEncoder(std::ifstream& is, std::ofstream& os, const std::string& magicNumber = "HUFFMAN1") 
		: is_(is), os_(os), bw_(os), magicNumber_(magicNumber), tableEntries_(0), huffmanTable_(), numSymbols_(0), data_() {}

	void compress() {
		std::map<uint8_t, size_t> frequencies = computeFrequencies();
		tableEntries_ = frequencies.size();
		Node* root = createHuffmanTree(frequencies);
		createHuffmanTable(root, 0, 0);
		computeNumSymbols();
		loadData();
		print();
	}

	std::map<uint8_t, size_t> computeFrequencies() {
		is_.seekg(0, std::ios::beg);
		is_.clear();
		uint8_t sym;
		std::map<uint8_t, size_t> frequencies;
		while (is_.read(reinterpret_cast<char*>(&sym), sizeof(uint8_t))) {
			++frequencies[sym];
		}
		return frequencies;
	}

	Node* createHuffmanTree(const std::map<uint8_t, size_t>& frequencies) {
		std::vector<Node*> nodes;
		for (const auto& [sym, frequency] : frequencies) {
			Node* node = new Node(sym, frequency);
			nodes.push_back(node);
		}

		auto sortNodesPredicate = [](Node* a, Node* b) {
			return a->count_ > b->count_;
			};

		std::sort(nodes.begin(), nodes.end(), sortNodesPredicate);

		while (nodes.size() > 1) {
			Node* left = nodes.back();
			nodes.pop_back();
			Node* right = nodes.back();
			nodes.pop_back();

			Node* node = new Node(left, right);
			nodes.push_back(node);
			std::sort(nodes.begin(), nodes.end(), sortNodesPredicate);
		}

		return nodes.back();
	}

	void createHuffmanTable(Node* node, size_t code, uint8_t len) {
		if (node->left_ == nullptr) {
			huffmanTable_[node->sym_] = std::make_pair(len, code);
		}
		else {
			createHuffmanTable(node->left_, (code << 1) | 0, len + 1);
			createHuffmanTable(node->right_, (code << 1) | 1, len + 1);
		}
	}

	void computeNumSymbols() {
		is_.clear();
		is_.seekg(0, std::ios::end);
		numSymbols_ = static_cast<size_t>(is_.tellg());
		is_.seekg(0, std::ios::beg);
		is_.clear();
	}

	void loadData() {
		is_.seekg(0, std::ios::beg);
		is_.clear();
		uint8_t byte;
		while (is_.read(reinterpret_cast<char*>(&byte), sizeof(uint8_t))) {
			data_.push_back(byte);
		}
	}

	void print() {
		os_ << "HUFFMAN1";
		uint8_t tableEntries = (tableEntries_ == 256) ? 0 : static_cast<uint8_t>(tableEntries_);
		os_.write(reinterpret_cast<const char*>(&tableEntries), sizeof(uint8_t));
		for (const auto& [sym, repr] : huffmanTable_) {
			const auto& [len, code] = repr;
			bw_.writeSequence(sym, 8);
			bw_.writeSequence(len, 5);
			bw_.writeSequence(code, len);
		}
		bw_.writeSequence(numSymbols_, 32);
		for (const auto& x : data_) {
			const auto& [len, code] = huffmanTable_[x];
			bw_.writeSequence(code, len);
		}
	}
};

int main(int argc, char** argv) {
	if (argc != 4) {
		return 1;
	}

	std::string command = argv[1];
	std::string i_filename = argv[2];
	std::string o_filename = argv[3];

	std::ifstream is(i_filename, std::ios::binary);
	if (!is) {
		return 1;
	}
	std::ofstream os(o_filename, std::ios::binary);
	if (!os) {
		return 1;
	}

	if (command == "c") {
		HuffmanEncoder he(is, os);
		he.compress();
	}
	else if (command == "d") {
		HuffmanDecoder hd(is, os);
		hd.decompress();
	}
	else {
		return 1;
	}

	return 0;
}