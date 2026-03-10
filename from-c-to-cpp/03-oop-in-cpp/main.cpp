#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

static int cmp_int32(const void* a, const void* b) {
    int32_t ia = *(const int32_t*)a;
    int32_t ib = *(const int32_t*)b;
    return (ia > ib) - (ia < ib);
}

struct vector {
    int32_t* data_;
    size_t n_;
    size_t cap_;

    vector() {
        this->data_ = NULL;
        this->n_ = 0;
        this->cap_ = 0;
    }
    ~vector() {
        free(this->data_);
    }
    void push_back(int32_t val) {
        if (this->n_ == this->cap_) {
            size_t new_cap = (this->cap_ == 0) ? 1024 : this->cap_ * 2;
            int32_t* tmp = (int32_t*)realloc(this->data_, new_cap * sizeof(int32_t));
            this->data_ = tmp;
            this->cap_ = new_cap;
        }
        this->data_[this->n_] = val;
        this->n_++;
    }
    void sort() {
        qsort(this->data_, this->n_, sizeof(int32_t), cmp_int32);
    }
    size_t size() const {
        return this->n_;
    }
    int at(size_t pos) const {
        assert(pos < this->n_);
        return this->data_[pos];
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

    //vector *arr = new vector();
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
        //arr->push_back(val);
        arr.push_back(val);
    }

    fclose(fin);

    //arr->sort();
    arr.sort();

    FILE* fout = fopen(out_name, "w");
    if (!fout) {
        //delete arr;
        return 1;
    }
    //for (size_t i = 0; i < arr->size(); i++) {
    //    fprintf(fout, "%d\n", arr->at(i));
    //}
    for (size_t i = 0; i < arr.size(); i++) {
        fprintf(fout, "%d\n", arr.at(i));
    }
    fclose(fout);

    //delete arr;
    return 0;
}