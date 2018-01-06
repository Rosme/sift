#include <string>
#include <fstream>
#include <cassert>

#include <nlohmann/json.hpp>

namespace Core {

    nlohmann::json read_json_file(const std::string& fileName) {
        std::string content;
        std::string line;

        std::ifstream file(fileName);

        assert(file.is_open());

        while(std::getline(file, line)) {
            content += line;
        }

        return nlohmann::json::parse(content);
    }

    void write_json_file(const std::string& fileName, const nlohmann::json& json) {
        std::ofstream file(fileName);

        assert(file.is_open());

        file << json;
    }

}