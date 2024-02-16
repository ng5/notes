#pragma once
#include <string>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <iostream>
#include <tuple>
#include <unordered_map>
template <typename T, T... S, typename F> constexpr void for_sequence(std::integer_sequence<T, S...>, F &&f) {
    (static_cast<void>(f(std::integral_constant<T, S>{})), ...);
}
template <typename Class, typename T> struct PropertyImpl {
    constexpr PropertyImpl(T Class::*aMember, const char *aName) : member{aMember}, name{aName} {}
    using Type [[maybe_unused]] = T;
    T Class::*member;
    const char *name;
};

template <typename Class, typename T> constexpr auto property(T Class::*member, const char *name) {
    return PropertyImpl<Class, T>{member, name};
}

template <typename T> class IsSupported {
    typedef char one;
    struct two {
        char x[2];
    };

    template <typename C> static one test(decltype(&C::properties));
    template <typename C> static two test(...);

  public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};
template <typename T> struct IsSupportedVector {
    enum { value = false };
};
template <typename T> struct IsSupportedVector<std::vector<T>> {
    typedef char one;
    struct two {
        char x[2];
    };
    template <typename C> static one test(decltype(&C::properties));
    template <typename C> static two test(...);
    typedef T type;
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};
template <typename T> struct IsSupportedVectorMap {
    enum { value = false };
};
template <typename T> struct IsSupportedVectorMap<std::vector<std::unordered_map<std::string, T>>> {
    typedef T type;
    enum { value = true };
};
template <typename K> struct IsStringMap {
    enum { value = false };
};
template <> struct IsStringMap<std::unordered_map<std::string, std::string>> {
    enum { value = true };
};
template <typename K> struct IsStringDoubleMap {
    enum { value = false };
};
template <> struct IsStringDoubleMap<std::unordered_map<std::string, double>> {
    enum { value = true };
};
template <typename T> struct IsSupportedMap {
    enum { value = false };
};
template <typename T> struct IsSupportedMap<std::unordered_map<std::string, T>> {
    typedef char one;
    struct two {
        char x[2];
    };
    template <typename C> static one test(decltype(&C::properties));
    template <typename C> static two test(...);
    typedef T type;
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};
template <typename T> struct IsStringVectorMap {
    enum { value = false };
};
template <> struct IsStringVectorMap<std::unordered_map<std::string, std::vector<double>>> {
    enum { value = true };
};

template <typename T> struct IsString {
    enum { value = false };
};
template <> struct IsString<std::string> {
    enum { value = true };
};
template <typename T> struct IsJsonScalar {
    enum { value = (std::is_arithmetic_v<T> || IsString<T>::value) };
};
template <typename T> struct IsVector {
    enum { value = false };
};
template <typename T> struct IsVector<std::vector<T>> {
    typedef T type;
    enum { value = true };
};

