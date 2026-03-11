#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <utility>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <ostream>
#include <vector>
#include <print>
#include <iostream>
#include <ranges>
#include <string>

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

//// Function object or Functor
//struct myorder {
//    double avg;
//    bool operator()(int a, int b) const
//    {
//        auto da = abs(a - avg);
//        auto db = abs(b - avg);
//        return da < db;
//    }
//};

int main(int argc, char** argv)
{
    namespace rg = std::ranges;

    if (argc != 3) {
        return 1;
    }

    std::vector<int> arr = read_from_file(argv[1]);
    // sort based on distance from the average (80.71428571)
    double sum = accumulate(begin(arr), end(arr), 0.0);
    double avg = sum / arr.size();
    std::sort(arr.begin(), arr.end(),
        [&](int a, int b) {
            auto da = abs(a - avg);
            auto db = abs(b - avg);
            return da < db;
        }
    );
    //std::sort(arr.begin(), arr.end(), std::greater<int>());

    struct person {
        std::string name, surname;
        int age;
    };
    std::vector<person> pvec = {
        {"Marco", "Lugli", 17},
        {"Luca", "Rossi", 65},
        {"Francesco", "Ferrari", 41},
    };
    auto p = &person::age;
    rg::sort(pvec, {}, &person::surname);

    for (const auto& x : arr) {
        std::println("{}", x);
    }

    write_to_file(argv[2], arr);

    return 0;
}