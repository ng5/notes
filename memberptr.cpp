#define ANKERL_NANOBENCH_IMPLEMENT
#include <iostream>
#include <mapbox/eternal.hpp>
#include <nanobench/nanobench.h>
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

    constexpr static auto X = mapbox::eternal::hash_map<mapbox::eternal::string, PropertyImpl<S, int>>({{"f1", property(&S::f1)},
                                                                                                        {"f2", property(&S::f2)},
                                                                                                        {"f3", property(&S::f3)},
                                                                                                        {"f4", property(&S::f4)},
                                                                                                        {"f5", property(&S::f5)},
                                                                                                        {"f6", property(&S::f6)},
                                                                                                        {"f7", property(&S::f7)},
                                                                                                        {"f8", property(&S::f8)},
                                                                                                        {"f9", property(&S::f9)},
                                                                                                        {"f10", property(&S::f10)}});

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
    const static std::unordered_map<std::string, int S::*> M2;
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
        os << "f1: " << s.f1 << " f2: " << s.f2 << " f3: " << s.f3 << " f4: " << s.f4 << " f5: " << s.f5 << " f6: " << s.f6 << " f7: " << s.f7 << " f8: " << s.f8 << " f9: " << s.f9 << " f10: " << s.f10;
        return os;
    }
};

const std::unordered_map<std::string, int S::*> S::M2 = {{"f1", &S::f1},
                                                         {"f2", &S::f2},
                                                         {"f3", &S::f3},
                                                         {"f4", &S::f4},
                                                         {"f5", &S::f5},
                                                         {"f6", &S::f6},
                                                         {"f7", &S::f7},
                                                         {"f8", &S::f8},
                                                         {"f9", &S::f9},
                                                         {"f10", &S::f10}};


int main() {
    S s;
    ankerl::nanobench::Bench().minEpochIterations(1000000).run("static map access", [&] {
        std::string field = "f10";
        s.*S::M.at(field.c_str()) = 1;
        ankerl::nanobench::doNotOptimizeAway(s);
    });
    ankerl::nanobench::Bench().minEpochIterations(1000000).run("string member access", [&] {
        std::string field = "f10";
        s.apply(field, 1);
        ankerl::nanobench::doNotOptimizeAway(s);
    });
    ankerl::nanobench::Bench().minEpochIterations(1000000).run("unordered_map access", [&] {
        std::string field = "f10";
        s.*S::M2.at(field) = 1;
        ankerl::nanobench::doNotOptimizeAway(s);
    });

    ankerl::nanobench::Bench().minEpochIterations(1000000).run("direct access", [&] {
        s.f10 = 1;
        ankerl::nanobench::doNotOptimizeAway(s);
    });


    return 0;
}
