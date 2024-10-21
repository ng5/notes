#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
class ArgParser {
    static std::unordered_map<std::string, std::string> buildArgFile(const std::string &argFile) {
        std::unordered_map<std::string, std::string> fileArgs;
        std::fstream fs(argFile, std::ios::in | std::ios::out | std::ios::app);
        if (fs.is_open()) {
            std::string line, ignore, key, value;
            while (getline(fs, line)) {
                if (line.starts_with("#") || line.empty()) continue;
                std::stringstream ss{line};
                ss >> ignore >> key >> value;
                fileArgs[key] = value;
            }
        }
        return fileArgs;
    }
    std::unordered_map<std::string, std::string> parsedArgs;
    std::string helpMsg;

public:
    auto help() {
        return helpMsg;
    }
    auto &get(const std::string &key) {
        return parsedArgs[key];
    }
    const auto &getAll() {
        return parsedArgs;
    }
    ArgParser(int argc, char **argv, const std::string &appName, const std::string &appDesc, std::initializer_list<std::string> appArgs) {
        cxxopts::Options options{appName, appDesc};
        for (const auto &key: appArgs) {
            options.add_options()(key, key, cxxopts::value<std::string>());
        }
        options.add_options()("h,help", "Print usage");
        options.allow_unrecognised_options();
        auto result = options.parse(argc, argv);
        helpMsg = options.help();
        std::string initFile;
        for (const auto &arg: result) {
            if (arg.key() == "help") continue;
            if (arg.key() == "INITFILE") {
                initFile = arg.value();
                continue;
            }
            parsedArgs[arg.key()] = arg.value();
        }
        if (!initFile.empty()) {
            auto fileArgs = buildArgFile(initFile);
            for (const auto &[k, v]: fileArgs) {
                if (!parsedArgs.contains(k))
                    parsedArgs[k] = v;
            }
        }
        bool helpShown = false;
        for (const auto &k: appArgs) {
            if (k == "INITFILE") continue;
            if (!parsedArgs.contains(k)) {
                if (!helpShown) {
                    std::cout << helpMsg << std::endl;
                    std::cerr << "missing argument(s):";
                    helpShown = true;
                }
                std::cerr << k << " ";
            }
        }
        if (helpShown) {
            std::cerr << std::endl;
            exit(1);
        }
    }
};
int main(const int argc, char **argv) {
    try {
        ArgParser argsParser{argc, argv, "ArgParser", "Merge command line and file parameters", {"MKVHOST", "MKVPORT", "MKVUSER", "MKVPWD", "INITFILE"}};
        const auto &all = argsParser.getAll();
        for (const auto &[k, v]: all)
            std::cout << k << ":" << v << std::endl;
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
