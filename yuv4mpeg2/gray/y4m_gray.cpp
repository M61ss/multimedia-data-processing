#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "mat.h"

struct SteamHeader {
	int W_;
	int H_;
	std::string C_ = "420jpeg";
	char I_ = 'p';
	std::pair<int, int> F_;
	std::pair<int, int> A_;
	std::string X_ = "";
};

struct FrameHeader {
	char I_;
	std::string X_ = "";
};

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string steamHeader;
	std::getline(is, steamHeader);
	std::stringstream ss(steamHeader);
	while (ss) {

	}

	return true;
}