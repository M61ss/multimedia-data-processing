#include <fstream>
#include <iostream>
#include <vector>
#include <map>

class Element {
private:
	char type_;

public:
	Element(const char& type = 0) : type_(type) {}

	virtual void read(std::istream& is);
};

class String : public Element {
private:
	std::string str_;

public:
	String() : Element('s'), str_("") {}

	void read(std::istream& is) override {
		size_t len = 0;
		is >> len;
		is.get();	// :
		for (size_t i = 0; i < len; i++) {
			char c = static_cast<const char>(is.get());
			str_.append(&c);
		}
	}
};

class Integer : public Element {
private:
	int64_t val_;

public:
	Integer() : Element('i'), val_(0) {}

	void read(std::istream& is) override {
		is.get();	// i
		is >> val_;
		is.get();	// e
	}
};

class List : public Element {
private: 
	std::vector<Element> list_;

public:
	List() : Element('l'), list_() {}

	void read(std::istream& is) override {
		is.get();	// l
		char next = is.peek();
		while (next != 'e') {
			if (next >= '0' && next <= '9') {
				String s;
				s.read(is);
				list_.push_back(s);
			}
			else if (next == 'i') {
				Integer i;
				i.read(is);
				list_.push_back(i);
			}
			else if (next == 'l') {
				List l;
				l.read(is);
				list_.push_back(l);
			}
			next = is.peek();
		}
		is.get();	// e
	}
};

class Dict : public Element {
private:
	std::map<String, Element> dict_;

public:
	Dict() : Element('d'), dict_() {}

	void read(std::istream& is) override {
		is.get();	// d
		char next = is.peek();
		while (next != 'e') {
			String key; 
			key.read(is);
			next = is.peek();
			if (next >= '0' && next <= '9') {
				String s;
				s.read(is);
				dict_[key] = s;
			}
			else if (next == 'i') {
				Integer i;
				i.read(is);
				dict_[key] = i;
			}
			else if (next == 'l') {
				List l;
				l.read(is);
				dict_[key] = l;
			}
			else if (next == 'd') {
				Dict d;
				d.read(is);
				dict_[key] = d;
			}
			next = is.peek();
		}
		is.get();	// e
	}
};

int main(int argc, char** argv) {
	if (argc != 2) {
		return -1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return -1;
	}

	std::vector<Element> elems;
	char next = is.peek();
	while (is) {
		if (next >= '0' && next <= '9') {
			String s;
			s.read(is);
			elems.push_back(s);
		}
		else if (next == 'i') {
			Integer i;
			i.read(is);
			elems.push_back(i);
		}
		else if (next == 'l') {
			List l;
			l.read(is);
			elems.push_back(l);
		}
		else if (next == 'd') {
			Dict d;
			d.read(is);
			elems.push_back(d);
		}
		next = is.peek();
	}

	return 0;
}