#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <utility>
#include <algorithm>
#include <fstream>
#include <ostream>

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
            std::copy_n(other.data_, n_, data_);
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

        // Could be improved by using "placement new"
        void push_back(const T& val) {
            if (n_ == cap_) {
                size_t new_cap = (cap_ == 0) ? 4 : cap_ * 2;
                T* tmp = new T[new_cap];
                std::copy_n(data_, n_, tmp);
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

        friend void swap(vector<T>& left, vector<T>& right) {
            std::swap(left.n_, right.n_);
            std::swap(left.cap_, right.cap_);
            std::swap(left.data_, right.data_);
        }
    };

}

mdp::vector<int> read_from_file(const char* filename) {
    mdp::vector<int> v;
    std::ifstream is(filename/*, std::ios::binary*/);
    if (!is) {
        return v;
    }
    int val;
    while (is >> val) {
        v.push_back(val);
    }
    return v;
}

bool write_to_file(const char* filename, const mdp::vector<int>& arr) {
    std::ofstream os(filename/*, std::ios::binary*/);
    if (!os) {
        return false;
    }
    for (size_t i = 0; i < arr.size(); i++) {
        //os << arr[i] << "\n";
        std::println(os, "{}", arr[i]);
    }
    return true;
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        return 1;
    }

    mdp::vector<int> arr;
    arr = read_from_file(argv[1]);

    mdp::vector<int> x = arr;

    //    arr.sort();

    write_to_file(argv[2], arr);

    return 0;
}