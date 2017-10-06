#define NOMINMAX

#include "FileUtils.h"
#include "FilterIndexReader.h"
#include "LuceneHeaders.h"
#include "MiscUtils.h"
#include "targetver.h"
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
#define MAX_RECORDS 4000000
#define RESULTS 20
using namespace Lucene;
using mongocxx::collection;
using mongocxx::cursor;
std::wstring_convert<std::codecvt_utf8<wchar_t>> CONVERTER;
mongocxx::options::find opts{};

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

void indexDB(const DirectoryPtr& ram, const std::string& url)
{
    auto start = std::chrono::high_resolution_clock::now();
    setOptions(MAX_RECORDS);
    auto analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr writer = newLucene<IndexWriter>(ram, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
    size_t rows = 0;
    auto lambda = [&writer, &rows](auto&& x) {
        for (const auto& r : x) {
            try {
                auto doc = newLucene<Document>();
                const std::string& country = r["Country"].get_utf8().value.to_string();
                const std::string& city = r["City"].get_utf8().value.to_string();
                auto countryPtr = newLucene<Field>(L"Country", CONVERTER.from_bytes(country), Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS);
                auto cityPtr = newLucene<Field>(L"City", CONVERTER.from_bytes(city), Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS);
                doc->add(countryPtr);
                doc->add(cityPtr);
                writer->addDocument(doc);
                ++rows;
            } catch (const std::exception& e) {
                std::cout << rows << ":" << e.what() << std::endl;
            }
        }
    };
    mongocxx::instance instance{};
    mongocxx::pool pool{ mongocxx::uri{ url } };
    dbcallfetch(MAX_RECORDS, pool, "dev_gini45", "CITY", "{}", "", lambda);
    std::cout << "indexed [" << rows << "] documents" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "index time(ms)=" << duration.count() << std::endl;
    start = std::chrono::high_resolution_clock::now();
    writer->optimize();
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "optimize time(ms)=" << duration.count() << std::endl;
    writer->close();
}

int main(int argc, char** argv)
{
    const std::string url = argv[1];
    auto ram = newLucene<RAMDirectory>();
    indexDB(ram, url);
    auto reader = IndexReader::open(ram, true);
    auto searcher = newLucene<IndexSearcher>(reader);
    Collection<String> fields = newCollection<String>(L"City");
    auto analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    auto parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"City", analyzer);
    parser->setEnablePositionIncrements(true);

    while (true) {
        try {
            std::string input;
            std::cout << "search: ";
            std::cin >> input;
            auto start = std::chrono::high_resolution_clock::now();
            std::wstring search = CONVERTER.from_bytes(input);
            auto collector = TopScoreDocCollector::create(RESULTS, false);
            auto query = parser->parse(search);
            searcher->search(query, collector);
            auto hits = collector->topDocs()->scoreDocs;
            uint32_t numHits = std::min(RESULTS, collector->getTotalHits());
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            for (uint32_t index = 0; index < numHits; ++index) {
                auto doc = searcher->doc(hits[index]->doc);
                if (doc.get()) {
                    std::wcout << doc->get(L"Country") << "\t" << doc->get(L"City") << std::endl;
                }
            }
            std::cout << "search time(us)=" << duration.count() << std::endl;

        } catch (const std::exception& e) {
            std::cout << "search error"
                      << ":" << e.what() << std::endl;
        }
    }
    return 0;
}
