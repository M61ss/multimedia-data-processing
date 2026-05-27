#include <fstream>

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

PDBheader readHeader(std::istream& is) {
	PDBheader hdr;
	is.read(hdr.name_.data(), 32);
	is.read(reinterpret_cast<char*>(&hdr.attr1_), 1);
	is.read(reinterpret_cast<char*>(&hdr.attr2_), 1);
	is.read(reinterpret_cast<char*>(&hdr.version_), 2);
	is.read(reinterpret_cast<char*>(&hdr.creationDate_), 4);
	is.read(reinterpret_cast<char*>(&hdr.modificationDate_), 4);
	is.read(reinterpret_cast<char*>(&hdr.lastBackupDate_), 4);
	is.read(reinterpret_cast<char*>(&hdr.modificationNumber_), 4);
	is.read(reinterpret_cast<char*>(&hdr.appInfoID_), 4);
	is.read(reinterpret_cast<char*>(&hdr.sortInfoID_), 4);
	is.read(hdr.type_.data(), 4);
	is.read(hdr.creator_.data(), 4);
	is.read(reinterpret_cast<char*>(&hdr.uniqueIDseed_), 4);
	is.read(reinterpret_cast<char*>(&hdr.nextRecordListID_), 4);
	is.read(reinterpret_cast<char*>(&hdr.numberOfRecords_), 2);

	return hdr;
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

	PDBheader hdr = readHeader(is);

	return 0;
}