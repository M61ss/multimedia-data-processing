#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

static int cmp_int32(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
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

    int* arr = NULL;
    size_t n = 0;
    size_t cap = 0;

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

        if (n == cap) {
            size_t new_cap = (cap == 0) ? 1024 : cap * 2;
            int* tmp = (int*)realloc(arr, new_cap * sizeof(*arr));
            arr = tmp;
            cap = new_cap;
        }

        arr[n] = (int)val;
        n++;
    }

    fclose(fin);

    qsort(arr, n, sizeof(*arr), cmp_int32);

    FILE* fout = fopen(out_name, "w");
    if (!fout) {
        free(arr);
        return 1;
    }
    for (size_t i = 0; i < n; i++) {
        fprintf(fout, "%d\n", arr[i]);
    }
    fclose(fout);

    free(arr);
    return 0;
}