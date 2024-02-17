#pragma once
#include <string>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <armadillo>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <serverlib/typeutil.h>
namespace serverlib::Json {
std::string prettyPrint(const rapidjson::Document &d);
std::string prettyPrint(const std::string &json);
std::string serialize(const rapidjson::Document &d);

template <typename W> void insert(W &w, float t) { w.Double(t); }
template <typename W> void insert(W &w, double t) { w.Double(t); }
template <typename W> void insert(W &w, bool t) { w.Bool(t); }
template <typename W> void insert(W &w, int t) { w.Int(t); }
template <typename W> void insert(W &w, uint32_t t) { w.Uint(t); }
template <typename W> void insert(W &w, int64_t t) { w.Int64(t); }
template <typename W> void insert(W &w, size_t t) { w.Uint64(t); }
template <typename W> void insert(W &w, const std::string &t) { w.String(t.c_str()); }

template <typename W> [[maybe_unused]] void write(W &w, const arma::mat &matrix, const char *name) {
    w.Key(name);
    w.StartArray();
    for (int i = 0; i < matrix.n_rows; ++i) {
        w.StartArray();
        for (int j = 0; j < matrix.n_cols; ++j) {
            insert(w, matrix.at(i, j));
        }
        w.EndArray(matrix.n_cols);
    }
    w.EndArray(matrix.n_rows);
}
template <typename W> [[maybe_unused]] void write(W &w, const arma::rowvec &vec, const char *name) {
    w.Key(name);
    w.StartArray();
    for (int i = 0; i < vec.n_cols; ++i) {
        insert(w, vec.at(0, i));
    }
    w.EndArray(vec.n_cols);
}
template <typename W> [[maybe_unused]] void write(W &w, const arma::colvec &vec, const char *name) {
    w.Key(name);
    w.StartArray();
    for (int i = 0; i < vec.n_rows; ++i) {
        insert(w, vec.at(i, 0));
    }
    w.EndArray(vec.n_rows);
}
template <typename W, typename T>
[[maybe_unused]] std::enable_if_t<!std::is_array_v<T>, void> write(W &w, const T &t, const char *name) {
    w.Key(name);
    insert(w, t);
}
template <typename W, typename T, size_t N> [[maybe_unused]] void write(W &w, T (&t)[N], const char *name) {
    w.Key(name);
    w.StartArray();
    for (auto i = 0; i < N; ++i)
        insert(w, t[i]);
    w.EndArray(N);
}
template <typename W, typename T> void write(W &w, const std::vector<T> &v, const char *name) {
    if (strlen(name) > 0)
        w.Key(name);
    w.StartArray();
    if constexpr (IsJsonScalar<T>::value) {
        for (auto i = 0; i < v.size(); ++i)
            insert(w, v[i]);
    } else if constexpr (IsVector<T>::value) {
        for (auto i = 0; i < v.size(); ++i)
            write(w, v[i], "");
    }
    w.EndArray(v.size());
}

template <typename W, typename T, typename F> void serialize(const std::string &prefix, W &w, const T &t, const F &f) {
    constexpr auto N = std::tuple_size<F>::value;
    w.StartObject();
    for_sequence(std::make_index_sequence<N>{}, [&](auto i) {
        auto property = std::get<i>(f);
        auto topKey = prefix + property.name;
        using Type = typename decltype(property)::Type;
        if constexpr (IsSupportedVector<Type>::value) {
            using K = typename IsSupportedVector<Type>::type;
            const std::vector<K> &v = t.*(property.member);
            w.Key(topKey.c_str());
            w.StartArray();
            for (const auto &item : v) {
                serialize("", w, item, std::remove_reference_t<decltype(item)>::properties);
            }
            w.EndArray(v.size());
        } else if constexpr (IsSupportedMap<Type>::value) {
            using K = typename IsSupportedMap<Type>::type;
            const std::unordered_map<std::string, K> &m = t.*(property.member);
            w.Key(topKey.c_str());
            w.StartObject();
            for (auto &r : m) {
                w.Key(r.first.c_str());
                serialize("", w, r.second, std::remove_reference_t<decltype(r.second)>::properties);
            }
            w.EndObject();
        } else if constexpr (IsSupported<Type>::value) {
            w.Key(topKey.c_str());
            serialize("", w, t.*(property.member), std::remove_reference_t<decltype(t.*(property.member))>::properties);
        } else if constexpr (IsStringMap<Type>::value) {
            const std::unordered_map<std::string, std::string> &m = t.*(property.member);
            w.Key(topKey.c_str());
            w.StartObject();
            for (auto &r : m) {
                w.Key(r.first.c_str());
                w.String(r.second.c_str());
            }
            w.EndObject();
        } else if constexpr (IsStringDoubleMap<Type>::value) {
            const std::unordered_map<std::string, double> &m = t.*(property.member);
            w.Key(topKey.c_str());
            w.StartObject();
            for (auto &r : m) {
                w.Key(r.first.c_str());
                w.Double(r.second);
            }
            w.EndObject();
        } else if constexpr (IsStringVectorMap<Type>::value) {
            const std::unordered_map<std::string, std::vector<double>> &m = t.*(property.member);
            w.Key(topKey.c_str());
            w.StartObject();
            for (auto &[r, v] : m) {
                w.Key(r.c_str());
                w.StartArray();
                for (const auto &i1 : v) {
                    w.Double(i1);
                }
                w.EndArray();
            }
            w.EndObject();
        } else if constexpr (IsSupportedVectorMap<Type>::value) {
            using K = typename IsSupportedVectorMap<Type>::type;
            const std::vector<std::unordered_map<std::string, K>> &v = t.*(property.member);
            w.Key(topKey.c_str());
            w.StartArray();
            for (const auto &mapRow : v) {
                w.StartObject();
                for (auto &[key, value] : mapRow) {
                    w.Key(key.c_str());
                    insert(w, value);
                }
                w.EndObject();
            }
            w.EndArray(v.size());
        } else {
            write(w, t.*(property.member), topKey.c_str());
        }
    });
    w.EndObject();
}
template <typename W, typename T, typename F> void serialize(W &w, const T &t, const F &f) { serialize("", w, t, f); }

template <typename W, typename T> void serialize(W &w, const T &t) { serialize(w, t, T::properties); }

// std::string serialize(const arma::mat &matrix);
template <typename T> std::string serializePretty(const T &t) {
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> w(sb);
    Json::serialize(w, t);
    return sb.GetString();
}
template <typename T> std::string serializePretty(const std::vector<T> &v) {
    if (v.empty())
        return "[]";
    std::string s = "[";
    for (const auto &t : v) {
        if constexpr (IsString<T>::value) {
            s += "\"" + t + "\"";
        } else if constexpr (std::is_floating_point_v<T> || std::is_integral_v<T>) {
            s += std::to_string(t);
        } else {
            s += serializePretty(t);
        }
        s += ",";
    }
    s.pop_back();
    s += "]";
    return s;
}
template <typename W, typename T> void serialize(W &w, const std::unordered_map<std::string, T> &v) {
    w.StartObject();
    for (auto &t : v) {
        w.Key(t.first.c_str());
        if constexpr (IsSupported<T>::value) {
            w.String(serialize(t.second));
        } else if constexpr (std::is_floating_point_v<T>) {
            w.Double(t.second);
        } else if constexpr (std::is_integral_v<T>) {
            w.Int64(t.second);
        } else {
            w.String(t.second);
        }
    }
    w.EndObject();
}

template <typename T> std::string serialize(const T &t) {
    using namespace rapidjson;
    StringBuffer sb;
    Writer<StringBuffer> w(sb);
    Json::serialize(w, t);
    return sb.GetString();
}
template <typename T> std::string serialize(const std::unordered_map<std::string, T> &v) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    serialize(writer, v);
    return buffer.GetString();
}

template <typename T> std::string serialize(const std::vector<T> &v) {
    if (v.empty())
        return "[]";
    std::string s = "[";
    for (const auto &t : v) {
        if constexpr (IsString<T>::value) {
            s += "\"" + t + "\"";
        } else if constexpr (std::is_floating_point_v<T>) {
            s += std::to_string(t);
        } else if constexpr (std::is_integral_v<T>) {
            s += std::to_string(t);
        } else {
            s += serialize(t);
        }
        s += ",";
    }
    s.pop_back();
    s += "]";
    return s;
}

template <typename W, typename T> void serialize(W &w, const std::unordered_map<std::string, std::vector<T>> &m) {
    w.StartObject();
    for (const auto &[k, v] : m) {
        w.Key(k.c_str());
        w.String(serialize(v));
    }
    w.EndObject();
}

template <typename T> std::string serialize(const std::unordered_map<std::string, std::vector<T>> &m) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    serialize(writer, m);
    return buffer.GetString();
}

[[maybe_unused]] void read(bool &t, const rapidjson::Value &d);
[[maybe_unused]] void read(int &t, const rapidjson::Value &d);
[[maybe_unused]] void read(int64_t &t, const rapidjson::Value &d);
[[maybe_unused]] void read(uint32_t &t, const rapidjson::Value &d);
[[maybe_unused]] void read(size_t &t, const rapidjson::Value &d);
[[maybe_unused]] void read(std::string &t, const rapidjson::Value &d);
[[maybe_unused]] void read(float &t, const rapidjson::Value &d);
[[maybe_unused]] void read(double &t, const rapidjson::Value &d);
template <typename T> void read(std::vector<T> &v, const rapidjson::Value &d) {
    if (!d.IsArray())
        return;
    v.clear();
    auto size = (size_t)d.GetArray().Size();
    v.reserve(size);
    for (auto i = 0; i < size; ++i) {
        T t;
        read(t, d.GetArray()[i]);
        v.emplace_back(t);
    }
}
template <typename T, size_t N> void read(T (&t)[N], const rapidjson::Value &d) {
    if (!d.IsArray())
        return;
    auto size = std::min(N, (size_t)d.GetArray().Size());
    for (auto i = 0; i < size; ++i) {
        read(t[i], d.GetArray()[i]);
    }
}
template <typename T> void deserialize(T &t, const rapidjson::Value &d) {
    constexpr auto nbProperties = std::tuple_size<decltype(T::properties)>::value;
    for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i) {
        constexpr auto property = std::get<i>(T::properties);
        using Type = typename decltype(property)::Type;
        if (d.HasMember(property.name)) {
            if constexpr (IsSupportedVector<Type>::value) {
                if (d[property.name].IsArray()) {
                    using K = typename IsSupportedVector<Type>::type;
                    std::vector<K> &v = t.*(property.member);
                    auto size = d[property.name].GetArray().Size();
                    v.reserve(size);
                    for (const auto &r : d[property.name].GetArray()) {
                        K k;
                        deserialize(k, r);
                        v.emplace_back(k);
                    }
                }
            } else if constexpr (IsStringMap<Type>::value) {
                if (!d[property.name].IsObject())
                    return;
                std::unordered_map<std::string, std::string> &m = t.*(property.member);
                m.clear();
                for (auto &r : d[property.name].GetObject()) {
                    m[r.name.GetString()] = d[property.name][r.name.GetString()].GetString();
                }
            } else if constexpr (IsSupportedMap<Type>::value) {
                if (!d[property.name].IsObject())
                    return;
                using K = typename IsSupportedMap<Type>::type;
                std::unordered_map<std::string, K> &m = t.*(property.member);
                m.clear();
                for (auto &r : d[property.name].GetObject()) {
                    K k;
                    deserialize(k, d[property.name][r.name.GetString()]);
                    m[r.name.GetString()] = k;
                }

            } else if constexpr (IsSupported<Type>::value) {
                deserialize(t.*(property.member), d[property.name]);
            } else {
                read(t.*(property.member), d[property.name]);
            }
        }
    });
}
template <typename T> void deserialize(T &t, const std::string &s) {
    using namespace rapidjson;
    Document d;
    auto parseError = d.Parse(s.c_str()).HasParseError();
    if (!parseError && d.IsObject())
        deserialize(t, d);
}
template <typename T> void deserialize(std::vector<T> &v, const rapidjson::Value &d) {
    if (!d.IsArray())
        return;
    v.clear();
    auto size = d.GetArray().Size();
    v.reserve(size);
    for (const auto &r : d.GetArray()) {
        if constexpr (IsString<T>::value) {
            if (r.IsString()) {
                v.emplace_back(r.GetString());
            }
        } else {
            T t;
            deserialize(t, r);
            v.emplace_back(t);
        }
    }
}
template <typename T> void deserialize(std::unordered_map<std::string, T> &m, const rapidjson::Value &d) {
    if (!d.IsObject())
        return;
    m.clear();
    for (auto &r : d.GetObject()) {
        T t;
        deserialize(t, d[r.name.GetString()]);
        m[r.name.GetString()] = t;
    }
}
template <typename T> void deserialize(std::vector<T> &v, const std::string &s) {
    using namespace rapidjson;
    Document d;
    auto parseError = d.Parse(s.c_str()).HasParseError();
    if (!parseError && d.IsArray())
        deserialize(v, d);
}

} // namespace serverlib::Json
