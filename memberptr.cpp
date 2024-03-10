#define ANKERL_NANOBENCH_IMPLEMENT
#include "include/types.h"
#include "mapbox/eternal.hpp"
#include "nanobench/nanobench.h"
#include <benchmark/benchmark.h>
#include <iostream>
#include <ostream>
#include <variant>

template<typename Class, typename T>
struct PropertyImpl {
    constexpr explicit PropertyImpl(T Class::*aMember) : member{aMember} {}
    using Type [[maybe_unused]] = T;
    T Class::*member;
};

template<typename Class, typename T>
[[maybe_unused]] constexpr auto property(T Class::*member) {
    return PropertyImpl<Class, T>{member};
}

struct S {
    int f1{};
    int f2{};
    int f3{};
    int f4{};
    int f5{};
    int f6{};
    int f7{};
    int f8{};
    int f9{};
    int f10{};
    double f11{};

    constexpr static auto M = mapbox::eternal::hash_map<mapbox::eternal::string, int S::*>({{"f1", &S::f1},
                                                                                            {"f2", &S::f2},
                                                                                            {"f3", &S::f3},
                                                                                            {"f4", &S::f4},
                                                                                            {"f5", &S::f5},
                                                                                            {"f6", &S::f6},
                                                                                            {"f7", &S::f7},
                                                                                            {"f8", &S::f8},
                                                                                            {"f9", &S::f9},
                                                                                            {"f10", &S::f10}});
    const static std::unordered_map<std::string, std::variant<int S::*, double S::*, std::string S::*>> M2;

    void apply(const std::string &key, int value) {
        if (key == "f1") f1 = value;
        if (key == "f2") f2 = value;
        if (key == "f3") f3 = value;
        if (key == "f4") f4 = value;
        if (key == "f5") f5 = value;
        if (key == "f6") f6 = value;
        if (key == "f7") f7 = value;
        if (key == "f8") f8 = value;
        if (key == "f9") f9 = value;
        if (key == "f10") f10 = value;
    }
    friend std::ostream &operator<<(std::ostream &os, const S &s) {
        os << "f1: " << s.f1 << " f2: " << s.f2 << " f3: " << s.f3 << " f4: " << s.f4 << " f5: " << s.f5 << " f6: " << s.f6 << " f7: " << s.f7 << " f8: " << s.f8 << " f9: " << s.f9 << " f10: " << s.f10 << " f11: " << s.f11;
        return os;
    }
};

const std::unordered_map<std::string, std::variant<int S::*, double S::*, std::string S::*>> S::M2 = {{"f1", &S::f1},
                                                                                                      {"f2", &S::f2},
                                                                                                      {"f3", &S::f3},
                                                                                                      {"f4", &S::f4},
                                                                                                      {"f5", &S::f5},
                                                                                                      {"f6", &S::f6},
                                                                                                      {"f7", &S::f7},
                                                                                                      {"f8", &S::f8},
                                                                                                      {"f9", &S::f9},
                                                                                                      {"f10", &S::f10},
                                                                                                      {"f11", &S::f11}};


namespace bm = benchmark;

static void static_map_access(bm::State &state) {
    S s{};
    for (auto _: state) {
        std::string field = "f7";
        s.*S::M.at(field.c_str()) = 1;
    }
}
static void direct_access(bm::State &state) {
    S s{};
    for (auto _: state)
        s.f7 = 1;
}
static void unordered_map_access(bm::State &state) {
    S s{};
    for (auto _: state) {
        std::string field = "f11";
        if (auto it = S::M2.find(field); it != S::M2.end()) {
            auto mptr = it->second;
            if (it->second.index() == 0) {
                s.*std::get<0>(it->second) = 1;
            } else if (it->second.index() == 1) {
                s.*std::get<1>(it->second) = 1.43323;
            } else if (it->second.index() == 2) {
                s.*std::get<2>(it->second) = "hello";
            }
        }
    }
}
static void string_access(bm::State &state) {
    S s{};
    for (auto _: state) {
        std::string field = "f7";
        s.apply(field, 1);
    }
}

BENCHMARK(static_map_access);
BENCHMARK(direct_access);
BENCHMARK(unordered_map_access);
BENCHMARK(string_access);


BENCHMARK_MAIN();
