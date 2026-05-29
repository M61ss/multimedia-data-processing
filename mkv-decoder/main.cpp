#include <fstream>
#include <vector>
#include <iostream>

class BitReader {
private:
	std::istream& is_;
	uint8_t buffer_;
	size_t n_;

public:
	BitReader(std::istream& is) : is_(is), buffer_(0), n_(0) {}

	uint8_t readBit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	size_t readSequence(const size_t& n) {
		size_t output = 0;
		for (size_t i = 0; i < n; i++) {
			output = (output << 1) | readBit();
		}
		return output;
	}
};

struct EBML {
	size_t ID_ = 0;
	size_t size_ = 0;
	std::vector<uint8_t> data_;
};

struct Segment {

};

struct Track {

};

struct Tag {

};

struct Cues {

};

struct Cluster {

};

struct MatroskaInfo {
	EBML ebml_;
	Segment segment_;
	std::vector<Track> tracks_;
	std::vector<Tag> tags_;
	Cues cues_;
	Cluster cluster_;
};

class MatroskaDecoder {
private:
	std::istream& is_;
	BitReader br_;
	MatroskaInfo mkInfo_;

	size_t readVariableSizeValue() {
		size_t count = 1;
		while (br_.readBit() == 0) {
			++count;
		}
		size_t val = br_.readSequence(8 * count - count);
		return val;
	}
	
	void readEBML() {
		mkInfo_.ebml_.ID_ = readVariableSizeValue();
		mkInfo_.ebml_.size_ = readVariableSizeValue();
		for (size_t i = 0; i < mkInfo_.ebml_.size_; i++) {
			uint8_t buff = 0;
			is_.read(reinterpret_cast<char*>(&buff), 1);
			mkInfo_.ebml_.data_.push_back(buff);
		}
	}

public:
	MatroskaDecoder(std::istream& is) : is_(is), br_(is) {}

	void retriveInfo() {
		readEBML();
	}

	const MatroskaInfo& mkInfo() const { return mkInfo_; }
};

void printInfo(std::ostream& os, const MatroskaInfo& mkInfo) {

}

int main(int argc, char** argv) {
	if (argc != 2) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}

	MatroskaDecoder mkDecoder(is);
	mkDecoder.retriveInfo();
	MatroskaInfo mkInfo = mkDecoder.mkInfo();
	printInfo(std::cout, mkInfo);

	return 0;
}