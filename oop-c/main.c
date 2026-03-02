#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>

static int cmp_int32(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

typedef struct vector {
    int* data_;
    size_t n_;
    size_t cap_;
} vector;
void vector_constructor(vector* this) {     // To put "vector_" as prefix it is called "namespacing"
    this->data_ = NULL;
    this->n_ = 0;
    this->cap_ = 0;
}
void vector_destructor(vector* this) {
    free(this->data_);
}
vector* new_vector(void) {
    vector* v = malloc(sizeof(vector));
    vector_constructor(v);
    return v;
}
void delete_vector(vector* v) {
    vector_destructor(v);
    free(v);
}
void vector_push_back(vector* this, int val) {
    if (this->n_ == this->cap_) {
        size_t new_cap = (this->cap_ == 0) ? 1024 : this->cap_ * 2;
        int* tmp = (int*)realloc(this->data_, new_cap * sizeof(*this->data_));
        this->data_ = tmp;
        this->cap_ = new_cap;
    }

    this->data_[this->n_] = val;
    this->n_++;
}
void vector_sort(vector* this) {
    qsort(this->data_, this->n_, sizeof(*this->data_), cmp_int32);
}
size_t vector_size(const vector* this) {
    return this->n_;
}
int vector_at(const vector* this, size_t pos) {
    assert(pos < this->n_);
    return this->data_[pos];
}

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

    vector* arr = new_vector();

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

        vector_push_back(arr, val);
    }

    fclose(fin);

    vector_sort(arr);

    FILE* fout = fopen(out_name, "w");
    if (!fout) {
        delete_vector(arr);
        return 1;
    }
    for (size_t i = 0; i < vector_size(arr); i++) {
        fprintf(fout, "%d\n", vector_at(arr, i));
    }
    fclose(fout);

    delete_vector(arr);
    return 0;
}