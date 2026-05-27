#include <fstream>
#include <iostream>
#include <vector>

struct PDBheader {
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
	std::string creator_ = 0;
	uint32_t uniqueIDseed_ = 0;
	uint32_t nextRecordListID_ = 0;
	uint16_t numberOfRecords_ = 0;

	PDBheader() : name_(32, 0), type_(4, 0), creator_(4, 0) {}
};

struct RecordInfoEntry {
	uint32_t recordDataOffset_;
	uint8_t recordAttributes_;
	uint32_t uniqueID_;
};

class MOBIdecoder {
private:
	std::istream& is_;
	std::ostream& os_;
	PDBheader hdr_;
	std::vector<RecordInfoEntry> ries_;

public:
	MOBIdecoder(std::istream& is, std::ostream& os) : is_(is), os_(os), hdr_(), ries_() {}

	void readHeader() {
		is_.read(hdr_.name_.data(), 32);
		is_.read(reinterpret_cast<char*>(&hdr_.attr1_), 1);
		is_.read(reinterpret_cast<char*>(&hdr_.attr2_), 1);
		is_.read(reinterpret_cast<char*>(&hdr_.version_), 2);
		is_.read(reinterpret_cast<char*>(&hdr_.creationDate_), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.modificationDate_), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.lastBackupDate_), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.modificationNumber_), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.appInfoID_), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.sortInfoID_), 4);
		is_.read(hdr_.type_.data(), 4);
		is_.read(hdr_.creator_.data(), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.uniqueIDseed_), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.nextRecordListID_), 4);
		is_.read(reinterpret_cast<char*>(&hdr_.numberOfRecords_), 2);
	}

	void printHeader() {
		os_ << 0xEF << 0xBB << 0xBF
			<< "PDB name: " << hdr_.name_ << "\n"
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
			is_.read(reinterpret_cast<char*>(&rie.recordAttributes_), 1);
			is_.read(reinterpret_cast<char*>(&rie.uniqueID_), 3);
			ries_.push_back(rie);
		}
	}

	void printRecordInfoEntries() {
		for (size_t i = 0; i < ries_.size(); i++) {
			os_ << i << " - offset: " << ries_[i].recordDataOffset_ << " - id: " << ries_[i].uniqueID_ << "\n";
		}
		os_ << "\n";
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

	MOBIdecoder md(is, std::cout);
	md.readHeader();
	md.printHeader();
	md.readRecordInfoEntries();
	md.printRecordInfoEntries();

	return 0;
}