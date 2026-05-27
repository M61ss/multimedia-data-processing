#include <fstream>
#include <iostream>
#include <vector>
#include <bit>

struct PDBHeader {
	std::string name_;
	uint8_t attr1_ = 0;
	uint8_t attr2_ = 0;
	uint16_t version_ = 0;
	uint32_t creationDate_ = 0;
	uint32_t modificationDate_ = 0;
	uint32_t lastBackupDate_ = 0;
	uint32_t modificationNumber_ = 0;
	uint32_t appInfoID_ = 0;
	uint32_t sortInfoID_ = 0;
	std::string type_;
	std::string creator_;
	uint32_t uniqueIDseed_ = 0;
	uint32_t nextRecordListID_ = 0;
	uint16_t numberOfRecords_ = 0;

	PDBHeader() : name_(32, 0), type_(4, 0), creator_(4, 0) {}
};

struct RecordInfoEntry {
	uint32_t recordDataOffset_;
	uint8_t recordAttributes_;
	uint32_t uniqueID_;
};

struct Record {
	uint16_t compression_ = 0;
	uint16_t unused_ = 0;
	uint32_t textLength_ = 0;
	uint16_t recordCount_ = 0;
	uint16_t recordSize_ = 0;
	uint16_t encryptionType_ = 0;
	uint16_t unknow_ = 0;
	std::vector<uint8_t> data_;
};

class MOBIDecoder {
private:
	std::istream& is_;
	std::ostream& os_;
	PDBHeader hdr_;
	std::vector<RecordInfoEntry> ries_;
	std::vector<Record> records_;

	void readPalmDOCHeader(Record& rcd) {
		is_.read(reinterpret_cast<char*>(&rcd.compression_), 2);
		rcd.compression_ = std::byteswap(rcd.compression_);
		is_.read(reinterpret_cast<char*>(&rcd.unused_), 2);
		rcd.unused_ = std::byteswap(rcd.unused_);
		is_.read(reinterpret_cast<char*>(&rcd.textLength_), 4);
		rcd.textLength_ = std::byteswap(rcd.textLength_);
		is_.read(reinterpret_cast<char*>(&rcd.recordCount_), 2);
		rcd.recordCount_ = std::byteswap(rcd.recordCount_);
		is_.read(reinterpret_cast<char*>(&rcd.recordSize_), 2);
		rcd.recordSize_ = std::byteswap(rcd.recordSize_);
		is_.read(reinterpret_cast<char*>(&rcd.encryptionType_), 2);
		rcd.encryptionType_ = std::byteswap(rcd.encryptionType_);
		is_.read(reinterpret_cast<char*>(&rcd.unknow_), 2);
		rcd.unknow_ = std::byteswap(rcd.unknow_);
	}

public:
	MOBIDecoder(std::istream& is, std::ostream& os) : is_(is), os_(os), hdr_(), ries_(), records_() {}

	void readHeader() {
		is_.read(hdr_.name_.data(), 32);
		is_.read(reinterpret_cast<char*>(&hdr_.attr1_), 1);
		is_.read(reinterpret_cast<char*>(&hdr_.attr2_), 1);
		is_.read(reinterpret_cast<char*>(&hdr_.version_), 2);
		hdr_.version_ = std::byteswap(hdr_.version_);
		is_.read(reinterpret_cast<char*>(&hdr_.creationDate_), 4);
		hdr_.creationDate_ = std::byteswap(hdr_.creationDate_);
		is_.read(reinterpret_cast<char*>(&hdr_.modificationDate_), 4);
		hdr_.modificationDate_ = std::byteswap(hdr_.modificationDate_);
		is_.read(reinterpret_cast<char*>(&hdr_.lastBackupDate_), 4);
		hdr_.lastBackupDate_ = std::byteswap(hdr_.lastBackupDate_);
		is_.read(reinterpret_cast<char*>(&hdr_.modificationNumber_), 4);
		hdr_.modificationNumber_ = std::byteswap(hdr_.modificationNumber_);
		is_.read(reinterpret_cast<char*>(&hdr_.appInfoID_), 4);
		hdr_.appInfoID_ = std::byteswap(hdr_.appInfoID_);
		is_.read(reinterpret_cast<char*>(&hdr_.sortInfoID_), 4);
		hdr_.sortInfoID_ = std::byteswap(hdr_.sortInfoID_);
		is_.read(hdr_.type_.data(), 4);
		is_.read(hdr_.creator_.data(), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.uniqueIDseed_), 4);
		hdr_.uniqueIDseed_ = std::byteswap(hdr_.uniqueIDseed_);
		is_.read(reinterpret_cast<char*>(&hdr_.nextRecordListID_), 4);
		hdr_.nextRecordListID_ = std::byteswap(hdr_.nextRecordListID_);
		is_.read(reinterpret_cast<char*>(&hdr_.numberOfRecords_), 2);
		hdr_.numberOfRecords_ = std::byteswap(hdr_.numberOfRecords_);
	}

	void printHeader() {
		size_t BOM = 0xBFBBEF;
		os_.write(reinterpret_cast<const char*>(&BOM), 3);
		os_ << "PDB name: " << hdr_.name_ << "\n"
			<< "Creation date (s): " << hdr_.creationDate_ << "\n"
			<< "Type: " << hdr_.type_ << "\n"
			<< "Creator: " << hdr_.creator_ << "\n"
			<< "Records: " << hdr_.numberOfRecords_ << "\n"
			<< "\n";
	}

	void readRecordInfoEntries() {
		for (uint16_t i = 0; i < hdr_.numberOfRecords_; i++) {
			RecordInfoEntry rie;
			is_.read(reinterpret_cast<char*>(&rie.recordDataOffset_), 4);
			rie.recordDataOffset_ = std::byteswap(rie.recordDataOffset_);
			is_.read(reinterpret_cast<char*>(&rie.recordAttributes_), 1);
			is_.read(reinterpret_cast<char*>(&rie.uniqueID_), 3);
			rie.uniqueID_ = std::byteswap(rie.uniqueID_);
			rie.uniqueID_ >>= 8;
			ries_.push_back(rie);
		}
	}

	void printRecordInfoEntries() {
		for (size_t i = 0; i < ries_.size(); i++) {
			os_ << i << " - offset: " << ries_[i].recordDataOffset_ << " - id: " << ries_[i].uniqueID_ << "\n";
		}
		os_ << "\n";
	}

	void readRecord(const size_t& n) {
		is_.seekg(ries_[n].recordDataOffset_);
		Record rcd;
		readPalmDOCHeader(rcd);

		for (uint16_t i = 0; i < rcd.recordCount_; i++) {
			uint8_t checkByte;
			while ((checkByte = is_.get()) != 0x00 || rcd.data_.size() == rcd.textLength_) {
				if (checkByte >= 0x01 && checkByte <= 0x08) {
					std::vector<uint8_t> asIs(8);
					is_.read(reinterpret_cast<char*>(asIs.data()), 8);
					for (const auto& byte : asIs) {
						rcd.data_.push_back(byte);
					}
				}
				else if (checkByte >= 0x09 && checkByte <= 0x7F) {
					rcd.data_.push_back(checkByte);
				}
				else if (checkByte >= 0x80 && checkByte <= 0xBF) {
					uint16_t buffer = checkByte;
					buffer <<= 8;
					buffer |= is_.get();
					uint16_t distance = buffer & 0b0011111111111000;
					distance >>= 3;
					uint8_t length = (buffer & 0x0007) + 3;
					auto range = rcd.data_.end() - distance;
					for (size_t i = 0; i < length; i++) {
						rcd.data_.push_back(*range);
						++range;
					}
				}
				else {
					rcd.data_.push_back(20);
					rcd.data_.push_back(checkByte & 0x7F);
				}

				if ((rcd.data_.size() % 4096) == 0) {
					break;
				}
			}
		}

		records_.push_back(rcd);
	}

	void printRecord(const size_t& n) {
		os_ << "Compression: " << records_[n].compression_ << "\n"
			<< "TextLength: " << records_[n].textLength_ << "\n"
			<< "RecordCount: " << records_[n].recordCount_ << "\n"
			<< "RecordSize: " << records_[n].recordSize_ << "\n"
			<< "EncryptionType: " << records_[n].encryptionType_ << "\n"
			<< "\n";
		os_.write(reinterpret_cast<const char*>(records_[n].data_.data()), records_[n].data_.size());
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

	MOBIDecoder md(is, os);
	md.readHeader();
	md.printHeader();
	md.readRecordInfoEntries();
	md.printRecordInfoEntries();
	md.readRecord(0);
	md.printRecord(0);

	return 0;
}