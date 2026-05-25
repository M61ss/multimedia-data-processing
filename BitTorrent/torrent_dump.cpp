#include <fstream>
#include <iostream>

void applyTabs(const size_t& tabs) {
	for (size_t i = 0; i < tabs; i++) {
		std::cout << "\t";
	}
}

int torrentDump(std::ifstream& is, char flag, size_t tabs, bool isDict, bool isValue) {
	if (flag == EOF) {
		return 0;
	}
	else if (flag == 'i') {
		char c;
		is.read(&c, sizeof(char));	// i
		is.read(&c, sizeof(char));
		applyTabs(tabs);
		while (c >= '0' && c <= '9') {
			std::cout << c;
			is.read(&c, sizeof(char));
		}
		is.read(&c, sizeof(char));	// e

		return torrentDump(is, static_cast<char>(is.peek()), tabs, isDict, isValue);
	}
	else if (flag == 'l') {
		char c;
		is.read(&c, sizeof(char));	// l
		if (isValue) {

		}
		std::cout << "[\n";

		torrentDump(is, static_cast<char>(is.peek()), tabs + 1, isDict, isValue);

		applyTabs(tabs);
		std::cout << "]\n";
		is.read(&c, sizeof(char));	// e
		
		return torrentDump(is, static_cast<char>(is.peek()), tabs - 1, isDict, isValue);
	}
	else if (flag == 'd') {
		char c;
		is.read(&c, sizeof(char));	// d
		std::cout << "{\n";

		torrentDump(is, static_cast<char>(is.peek()), tabs + 1, true, isValue);

		applyTabs(tabs);
		std::cout << "}\n";
		is.read(&c, sizeof(char));	// e

		return torrentDump(is, static_cast<char>(is.peek()), tabs - 1, false, isValue);
	}
	else if (flag >= '0' && flag <= '9') {
		int len = 0;
		is >> len;
		char c;
		is.read(&c, sizeof(char));	// :

		std::string str;
		for (int i = 0; i < len; i++) {
			is.read(&c, sizeof(char));
			str.append(&c);
		}

		if (isValue) {
			std::cout << " => ";
		}
		else {
			applyTabs(tabs);
		}
		std::cout << "\"" << str << "\"";

		return torrentDump(is, static_cast<char>(is.peek()), tabs, isDict, isValue);
	}
	else {
		return -1;
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		return -1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return -1;
	}

	return torrentDump(is, static_cast<char>(is.peek()), 0, false, false);
}