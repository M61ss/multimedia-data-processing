#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstdint>
#include <cassert>
#include <utility>

//static int cmp_int32(const void *a, const void *b) {
//    int32_t ia = *(const int32_t *)a;
//    int32_t ib = *(const int32_t *)b;
//    return (ia > ib) - (ia < ib);
//}

namespace mdp {

    template<typename T>
    class vector {
        T* data_;
        size_t n_;
        size_t cap_;
    public:
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
            data_ = new T[n];
        }
        // Copy constructor
        vector(const vector& other) { // const reference to vector
            n_ = other.n_;
            cap_ = other.cap_;
            data_ = new T[n_];
            for (size_t i = 0; i < n_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        // Move constructor
        vector(vector&& other) noexcept { // r-value reference to vector
            n_ = other.n_;
            cap_ = other.cap_;
            data_ = other.data_;
            other.data_ = nullptr;
        }
        // Assignment operator (Copy and Swap Idiom)
        vector& operator=(vector rhs) {
            using std::swap; // Enables ADL
            swap(*this, rhs);
            return *this;
        }
        ~vector() {
            delete[] data_;
        }

        void push_back(const T& val) {
            if (n_ == cap_) {
                size_t new_cap = (cap_ == 0) ? 4 : cap_ * 2;
                T* tmp = new T[new_cap];
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
        //void sort() {
        //    qsort(data_, n_, sizeof(int32_t), cmp_int32);
        //}
        size_t size() const {
            return n_;
        }
        const T& at(size_t pos) const {
            assert(pos < n_);
            return data_[pos];
        }
        const T& operator[](size_t pos) const {
            return data_[pos];
        }
        T& operator[](size_t pos) {
            return data_[pos];
        }

        // Option 1: declaration + definition
        // friend void swap(vector<T>& left, vector<T>& right);

        // Option 2: definition inside
        friend void swap(vector<T>& left, vector<T>& right) {
            std::swap(left.n_, right.n_);
            std::swap(left.cap_, right.cap_);
            std::swap(left.data_, right.data_);
        }
    };

    // Option 1: declaration + definition
    //template<typename T>
    //void swap(vector<T>& left, vector<T>& right) {
    //    std::swap(left.n_, right.n_);
    //    std::swap(left.cap_, right.cap_);
    //    std::swap(left.data_, right.data_);
    //}

}

mdp::vector<int> read_from_file(const char* filename) {
    FILE* fin = fopen(filename, "r");
    if (!fin) {
        return mdp::vector<int>();
    }

    mdp::vector<int> v;

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
        v.push_back(val);
    }
    fclose(fin);
    return v;
}

bool write_to_file(const char* filename, const mdp::vector<int>& arr) {
    FILE* fout = fopen(filename, "w");
    if (!fout) {
        return false;
    }
    for (size_t i = 0; i < arr.size(); i++) {
        fprintf(fout, "%d\n", arr[i]);
    }
    fclose(fout);
    return true;
}

struct person {
    char name[20], surname[20];
    int age;
};

int main(int argc, char** argv)
{
    namespace n = std; // Alias for namespace
    using std::swap;
    // using namespace std; // Don't do this

    int x = 5, y = 4;
    n::swap(x, y);

    double z = 5, t = 4;
    swap(z, t);

    person a = { "Enzo", "Catania", 32 };
    person b = { "Franco", "Rossi", 65 };
    swap(a, b);

    mdp::vector<int> c;
    c.push_back(1); c.push_back(2); c.push_back(3);
    mdp::vector<int> d;
    d.push_back(4); d.push_back(5); d.push_back(6);
    swap(c, d); // Argument-dependent lookup (ADL)
    //std::swap(c, d);
    //mdp::vector<int>::swap(c, d);

    if (argc != 3) {
        return 1;
    }

    const char* in_name = argv[1];
    const char* out_name = argv[2];

    mdp::vector<int> arr;
    arr = read_from_file(in_name);

    mdp::vector<double> darr;
    darr.push_back(3.5);

    mdp::vector<person> parr;
    parr.push_back({ "Mario", "Rossi", 41 });

    //    arr.sort();

    write_to_file(out_name, arr);

    return 0;
}