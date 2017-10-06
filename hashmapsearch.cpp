#define NOMINMAX
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <chrono>
#include <codecvt>
#include <exception>
#include <fstream>
#include <iostream>
#include <locale>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#define MAX_RECORDS 4000000
#define RESULTS 20

using mongocxx::collection;
using mongocxx::cursor;

std::wstring_convert<std::codecvt_utf8<wchar_t>> CONVERTER;
mongocxx::options::find opts{};
std::unordered_map<std::string, std::string> cityMap;

void setOptions(int max)
{
    opts.batch_size(max);
    opts.limit(max);
}

template <typename F, typename... ARGS>
decltype(auto) dbcallfetch(int max, mongocxx::pool& pool, const std::string& dbname, const std::string& table,
    const std::string& query, const std::string& fields, F f, ARGS&&... args)
{

    auto client = pool.acquire();
    bsoncxx::stdx::string_view view(query);
    auto filter = bsoncxx::from_json(view);

    if (!fields.empty()) {
        bsoncxx::stdx::string_view projection_view(fields);
        auto projection = bsoncxx::from_json(projection_view);
        opts.projection(static_cast<bsoncxx::document::value>(projection));
    }

    collection collection = (*client)[dbname][table];
    auto x = collection.find(static_cast<bsoncxx::document::value>(filter), opts);
    return f(x, std::forward<ARGS>(args)...);
}

void indexDB(const std::string url)
{
    auto start = std::chrono::high_resolution_clock::now();
    size_t rows = 0;
    auto lambda = [&rows](auto&& x) {
        for (const auto& r : x) {
            try {
                const std::string& country = r["Country"].get_utf8().value.to_string();
                const std::string& city = r["City"].get_utf8().value.to_string();
                cityMap.emplace(std::make_pair(city + "_" + country, country));
                ++rows;
            } catch (const std::exception& e) {
                std::cout << rows << ":" << e.what() << std::endl;
            }
        }
    };
    mongocxx::instance instance{};
    mongocxx::pool pool{ mongocxx::uri{ url} };
    dbcallfetch(MAX_RECORDS, pool, "dev_gini45", "CITY", "{}", "", lambda);
    std::cout << "indexed [" << rows << "] documents" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "index time(ms)=" << duration.count() << std::endl;
    start = std::chrono::high_resolution_clock::now();
}

int main(int argc, char** argv)
{
    const std::string url = argv[1];
    indexDB(url);
    while (true) {
        try {
            std::string input;
            std::cout << "search: ";
            std::cin >> input;
            auto start = std::chrono::high_resolution_clock::now();
            auto it = cityMap.find(input);
            auto end = std::chrono::high_resolution_clock::now();
            if (it != cityMap.end()) {
                std::cout << it->second << "\t" << it->first << std::endl;
            }

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "search time(us)=" << duration.count() << std::endl;

        } catch (const std::exception& e) {
            std::cout << "search error"
                      << ":" << e.what() << std::endl;
        }
    }
    return 0;
}
