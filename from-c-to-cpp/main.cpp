#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstdint>
#include <cassert>

static int cmp_int32(const void* a, const void* b) {
    int32_t ia = *(const int32_t*)a;
    int32_t ib = *(const int32_t*)b;
    return (ia > ib) - (ia < ib);
}

struct vector {
    int32_t* data_;
    size_t n_;
    size_t cap_;

    // Default constructor
    vector() {
        data_ = nullptr;
        n_ = 0;
        cap_ = 0;
    }
    // Constructor with starting size
    vector(size_t n) {
        n_ = n;
        cap_ = n;
        data_ = new int32_t[n];
    }
    // Copy constructor
    vector(const vector& other) {
        n_ = other.n_;
        cap_ = other.cap_;
        data_ = new int32_t[n_];
        for (size_t i = 0; i < n_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    // Assignment operator
    vector& operator=(const vector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        n_ = rhs.n_;
        cap_ = rhs.cap_;
        delete[] data_;
        data_ = new int32_t[n_];
        for (size_t i = 0; i < n_; ++i) {
            data_[i] = rhs.data_[i];
        }
        return *this;
    }

    ~vector() {
        delete[] data_;
    }
    void push_back(int32_t val) {
        if (n_ == cap_) {
            size_t new_cap = (cap_ == 0) ? 4 : cap_ * 2;
            int32_t* tmp = new int32_t[new_cap];
            for (size_t i = 0; i < n_; ++i) {
                tmp[i] = data_[i];
            }
            delete[] data_;
            data_ = tmp;
            cap_ = new_cap;
        }
        data_[n_] = val;
        n_++;
    }
    void sort() {
        qsort(data_, n_, sizeof(int32_t), cmp_int32);
    }
    size_t size() const {
        return n_;
    }
    int at(size_t pos) const {
        assert(pos < n_);
        return data_[pos];
    }
    int operator[](size_t pos) const {
        return data_[pos];
    }
};

int main(int argc, char** argv)
{
    if (argc != 3) {
        return 1;
    }

    const char* in_name = argv[1];
    const char* out_name = argv[2];

    FILE* fin = fopen(in_name, "r");
    if (!fin) {
        return 1;
    }

    vector arr;

    char token[128];
    while (fscanf(fin, "%127s", token) == 1) {
        errno = 0;
        char* endp = NULL;
        long val = strtol(token, &endp, 10);
        if (*endp != '\0') {
            break;
        }
        if (errno == ERANGE || val < INT32_MIN || val > INT32_MAX) {
            break;
        }
        arr.push_back(val);
    }

    fclose(fin);

    vector x(10); // vector x(arr); 
    x = arr;
    arr.sort();

    FILE* fout = fopen(out_name, "w");
    if (!fout) {
        return 1;
    }
    for (size_t i = 0; i < arr.size(); i++) {
        fprintf(fout, "%d\n", arr[i]);
    }
    fclose(fout);

    return 0;
}