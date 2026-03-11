#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <utility>
#include <algorithm>
#include <fstream>
#include <ostream>
#include <vector>
#include <print>
#include <iostream>

std::vector<int> read_from_file(const char* filename) {
    std::vector<int> v;
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

bool write_to_file(const char* filename, const std::vector<int>& arr) {
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

    std::vector<int> arr = read_from_file(argv[1]);

    // Option 1: old style direct indexing
    //for (size_t i = 0; i < arr.size(); ++i) {
    //    //std::cout << arr[i] << "\n";
    //    std::println("{}", arr[i]);
    //}

    // Option 2: iterators
    //std::vector<int>::iterator b = arr.begin();
    //std::vector<int>::iterator e = arr.end();
    //std::vector<int>::iterator it;
    //for (it = b; it != e; ++it) {
    //    std::println("{}", *it);
    //}

    // Option 3: iterators + auto + alias
    //for (auto it = arr.begin(), e = arr.end(); it != e; ++it) {
    //    const auto& x = *it;
    //    std::println("{}", x);
    //}

    // Option 4: range-based for
    for (const auto& x : arr) {
        std::println("{}", x);
    }


    //    arr.sort();

    write_to_file(argv[2], arr);

    return 0;
}