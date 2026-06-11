#include <fstream>
#include <vector>
#include <bit>
#include <cmath>

template <typename T>
bool rawRead(T* val, std::istream& is, const size_t& length) {
	return is.read(reinterpret_cast<char*>(val), length).good();
}

template <typename T>
bool rawWrite(const T* val, std::ostream& os, const size_t& length) {
	return os.write(reinterpret_cast<const char*>(val), length).good();
}

class BitReader {
private:
	std::istream& is_;
	uint8_t buffer_;
	uint8_t n_;

	uint8_t readBit() {
		if (n_ == 0) {
			rawRead(&buffer_, is_, 1);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	BitReader(std::istream& is) : is_(is), buffer_(0), n_(0) {}

	size_t readSequence(const size_t& length) {
		size_t val = 0;
		for (size_t i = 0; i < length; i++) {
			val = (val << 1) | readBit();
		}
		return val;
	}

	void flush() {
		while (n_ > 0) {
			readBit();
		}
	}
};

class FlacDecoder {
private:
	std::istream& is_;
	BitReader br_;
	std::string magicNumber_;
	size_t blockSize_ = 0;
	size_t bitDepth_ = 0;
	std::vector<int16_t> data_;

	void readHeader() {
		rawRead(magicNumber_.data(), is_, 4);
		if (magicNumber_ != "fLaC") {
			throw 1;
		}
	}

	void readMetadata() {
		size_t flag = br_.readSequence(1);
		size_t blockType = br_.readSequence(7);
		size_t blockSize = br_.readSequence(24);
		size_t currPos = is_.tellg();
		is_.seekg(currPos + blockSize);

		if (flag == 0) {
			readMetadata();
		}
	}

	size_t readCodedNumber() {
		size_t byteCount = 0;
		while (br_.readSequence(1) == 1) {
			byteCount++;
		}
		if (byteCount == 0) {
			return br_.readSequence(7);
		}

		size_t codedNumber = br_.readSequence(8 - (byteCount + 1));
		for (size_t i = 1; i < byteCount - 1; i++) {
			br_.readSequence(2);
			codedNumber = (codedNumber << 6) | br_.readSequence(6);
		}

		return codedNumber;
	}

	size_t readUnary() {
		size_t val = 0;
		val++;
		while (br_.readSequence(1) == 0) {
			val++;
		}
		return val;
	}

	void readParatition(const size_t& partitionLength, const size_t& riceParam) {
		for (size_t j = 0; j < partitionLength; j++) {
			size_t foldedResidual = (readUnary() << riceParam) | br_.readSequence(riceParam);
			int16_t unfoldedResidual = (foldedResidual % 2 == 0) ? foldedResidual / 2 : (foldedResidual + 1) / -2;
			data_.push_back(data_.back() + unfoldedResidual);
		}
	}

	void readSubframe() {
		size_t controlBit = br_.readSequence(1);
		if (controlBit != 0) {
			throw 1;
		}
		size_t predictorOrder = br_.readSequence(6);
		if (predictorOrder >= 0b001000 && predictorOrder <= 0b001100) {
			predictorOrder = predictorOrder - 8;
		}
		else {
			throw 1;
		}
		if (predictorOrder != 1) {
			throw 1;
		}
		size_t wastedFlag = br_.readSequence(1);
		size_t wastedBits = 0;
		if (wastedFlag == 1) {
			wastedBits = readUnary();
		}

		size_t nBitsperSample = bitDepth_ - wastedBits;
		data_.push_back(static_cast<int16_t>((br_.readSequence(nBitsperSample * predictorOrder) + 1)) / -2);
		size_t riceParamsBits = br_.readSequence(2);
		if (riceParamsBits == 0b00) {
			riceParamsBits = 4;
		}
		else if (riceParamsBits == 0b01) {
			riceParamsBits = 5;
		}
		else {
			throw 1;
		}
		size_t partitionOrder = br_.readSequence(4);
		size_t nPartitions = static_cast<size_t>(pow(2, partitionOrder));
		size_t partitionLength = blockSize_ >> partitionOrder;
		size_t riceParam = br_.readSequence(riceParamsBits);

		size_t nBitsPerResidual = 0;
		if (riceParam == 0x1F || riceParam == 0xF) {
			nBitsPerResidual = br_.readSequence(5);
			// ...
		}
		else {
			readParatition(partitionLength - predictorOrder, riceParam);
		}
		for (size_t i = 1; i < nPartitions; i++) {
			riceParam = br_.readSequence(riceParamsBits);
			if (riceParam == 0x1F || riceParam == 0xF) {
				nBitsPerResidual = br_.readSequence(5);
				// ...
			}
			else {
				readParatition(partitionLength, riceParam);
			}
		}
	}

	void readFrame() {
		size_t syncCode = br_.readSequence(15);
		if (syncCode != 0b111111111111100) {
			throw 1;
		}
		size_t blockingStrategy = br_.readSequence(1);
		if (blockingStrategy != 0) {
			throw 1;
		}

		blockSize_ = br_.readSequence(4);
		size_t sampleRate = br_.readSequence(4);
		if (sampleRate != 0b1001) {
			throw 1;
		}
		size_t nChannels = br_.readSequence(4);
		if (nChannels != 0b0000) {
			throw 1;
		}
		bitDepth_ = br_.readSequence(3);
		if (bitDepth_ != 0b100) {
			throw 1;
		}
		size_t reserved = br_.readSequence(1);
		if (reserved != 0) {
			throw 1;
		}
		readCodedNumber();

		if (blockSize_ == 0b0001) {
			blockSize_ = 192;
		}
		else if (blockSize_ == 0b0110) {
			blockSize_ = br_.readSequence(8) + 1;
		}
		else if (blockSize_ == 0b0111) {
			blockSize_ = br_.readSequence(16) + 1;
		}
		else if (blockSize_ >= 0b0010 && blockSize_ <= 0b0101) {
			blockSize_ = static_cast<size_t>(144 * pow(2, blockSize_));
		}
		else if (blockSize_ >= 0b1000 && blockSize_ <= 0b1111) {
			blockSize_ = static_cast<size_t>(pow(2, blockSize_));
		}
		else {
			throw 1;
		}

		size_t crc = br_.readSequence(8);
		readSubframe();
		br_.flush();
		size_t footer = br_.readSequence(16);
	}

public:
	FlacDecoder(std::istream& is) : is_(is), br_(is), magicNumber_(4, 0), data_() {}

	std::vector<int16_t> decode() {
		readHeader();
		readMetadata();
		readFrame();

		return data_;
	}
};

int main(int argc, char** argv) {
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return 1;
	}

	FlacDecoder flacd(is);
	std::vector<int16_t> data = flacd.decode();
	rawWrite(data.data(), os, data.size() * 2);

	return 0;
}