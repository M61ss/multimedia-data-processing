#include <fstream>
#include <vector>
#include <bit>
#include <cmath>

struct FileHeader {
	std::string magicNumber_;
	uint32_t samplesPerChannel_;

	FileHeader() : magicNumber_(4, 0), samplesPerChannel_(0) {}
};

struct FrameHeader {
	uint8_t nChannels_;
	uint32_t sampleRate_;
	uint16_t frameSamples_;
	uint16_t frameSize_;
};

struct LMSState {
	std::vector<int16_t> history_;
	std::vector<int16_t> weights_;

	LMSState() : history_(4), weights_(4) {}
};

class QOADecoder {
private:
	std::istream& is_;
	FileHeader fileHdr_;
	std::vector<double> dequantTab_ = { 0.75, -0.75, 2.5, -2.5, 4.5, -4.5, 7, -7 };
	std::vector<int16_t> data_;

	void readFileHeader() {
		is_.read(fileHdr_.magicNumber_.data(), 4);
		is_.read(reinterpret_cast<char*>(&fileHdr_.samplesPerChannel_), 4);
		fileHdr_.samplesPerChannel_ = std::byteswap(fileHdr_.samplesPerChannel_);
	}

	FrameHeader readFrameHeader() {
		FrameHeader frameHdr;
		is_.read(reinterpret_cast<char*>(&frameHdr.nChannels_), 1);
		is_.read(reinterpret_cast<char*>(&frameHdr.sampleRate_), 3);
		frameHdr.sampleRate_ = std::byteswap(frameHdr.sampleRate_);
		frameHdr.sampleRate_ >>= 8;
		is_.read(reinterpret_cast<char*>(&frameHdr.frameSamples_), 2);
		frameHdr.frameSamples_ = std::byteswap(frameHdr.frameSamples_);
		is_.read(reinterpret_cast<char*>(&frameHdr.frameSize_), 2);
		frameHdr.frameSize_ = std::byteswap(frameHdr.frameSize_);

		return frameHdr;
	}

	LMSState readLMSState() {
		LMSState lms;
		for (size_t i = 0; i < 4; i++) {
			int16_t hist = 0;
			is_.read(reinterpret_cast<char*>(&hist), 2);
			lms.history_[i] = std::byteswap(hist);
		}
		for (size_t i = 0; i < 4; i++) {
			int16_t weight = 0;
			is_.read(reinterpret_cast<char*>(&weight), 2);
			lms.weights_[i] = std::byteswap(weight);
		}

		return lms;
	}

	std::vector<uint8_t> extractSamples(const uint64_t& slice) {
		uint64_t mask = 0b0000111000000000000000000000000000000000000000000000000000000000;
		uint8_t shift = 57;
		std::vector<uint8_t> qr;
		for (size_t i = 0; i < 20; i++) {
			qr.push_back(static_cast<uint8_t>((slice & mask) >> shift));
			mask >>= 3;
			shift -= 3;
		}

		return qr;
	}

	void dequantize(const double& sf, const std::vector<uint8_t>& qr, LMSState& lms) {
		for (const auto& q : qr) {
			double residual = sf * dequantTab_[q];
			int32_t r = (residual < 0) ? static_cast<int32_t>(ceil(residual - 0.5)) : static_cast<int32_t>(floor(residual + 0.5));
			int32_t p = 0;
			for (size_t i = 0; i < 4; i++) {
				p += lms.history_[i] * lms.weights_[i];
			}
			p >>= 13;
			int16_t s = p + r;
			if (s > 32767) {
				s = 32767;
			}
			else if (s < -32768) {
				s = -32768;
			}
			data_.push_back(s);

			int16_t delta = r >> 4;
			for (size_t i = 0; i < 4; i++) {
				lms.weights_[i] += (lms.history_[i] < 0) ? -delta : delta;
			}
			for (size_t i = 0; i < 3; i++) {
				lms.history_[i] = lms.history_[i + 1];
			}
			lms.history_[3] = s;
		}
	}

public:
	QOADecoder(std::istream& is) : is_(is) {}

	void decode() {
		readFileHeader();
		const size_t frameNumber = static_cast<size_t>(ceil(fileHdr_.samplesPerChannel_ / (256 * 20)));

		for (size_t i = 0; i < frameNumber; i++) {
			FrameHeader frameHdr = readFrameHeader();

			std::vector<LMSState> lms(frameHdr.nChannels_);
			for (size_t c = 0; c < frameHdr.nChannels_; c++) {
				lms[c] = readLMSState();
			}
			
			for (size_t s = 0; s < frameHdr.frameSamples_ / 20; s++) {
				for (size_t c = 0; c < frameHdr.nChannels_; c++) {
					uint64_t slice = 0;
					is_.read(reinterpret_cast<char*>(&slice), 8);
					slice = std::byteswap(slice);

					uint8_t sfQuant = (slice & 0xF000000000000000) >> 60;
					std::vector<uint8_t> qr = extractSamples(slice);

					double sf = round(pow(sfQuant + 1, 2.75));

					dequantize(sf, qr, lms[c]);
				}
			}
		}
	}

	const std::vector<int16_t>& data() const { return data_; }
};

void saveWAV(std::ostream& os, const QOADecoder& qoa) {
	os << "RIFF";
	size_t fileSize = 0;
	os.write(reinterpret_cast<const char*>(&fileSize), 4);
	os << "WAVE" << "fmt ";
	int32_t length = 16;
	os.write(reinterpret_cast<const char*>(&length), 4);
	int16_t tof = 1;
	os.write(reinterpret_cast<const char*>(&tof), 2);
	int16_t nChannels = 2;
	os.write(reinterpret_cast<const char*>(&nChannels), 2);
	int32_t samplingRate = 44100;
	os.write(reinterpret_cast<const char*>(&samplingRate), 4);
	int32_t val1 = 176400;
	os.write(reinterpret_cast<const char*>(&val1), 4);
	uint16_t val2 = 4;
	os.write(reinterpret_cast<const char*>(&val2), 2);
	uint16_t bitPerSample = 16;
	os.write(reinterpret_cast<const char*>(&bitPerSample), 2);
	os << "data";
	size_t dataSize = qoa.data().size() * 2;
	os.write(reinterpret_cast<const char*>(&dataSize), 4);

	os.write(reinterpret_cast<const char*>(qoa.data().data()), dataSize);

	fileSize = os.tellp();
	os.seekp(4);
	os.write(reinterpret_cast<const char*>(&fileSize), 4);
}

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

	QOADecoder qoa(is);
	qoa.decode();
	saveWAV(os, qoa);

	return 0;
}