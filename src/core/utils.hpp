#include <string>
#include <fstream>
#include <cassert>

#include <nlohmann/json.hpp>
#include <muflihun/easylogging++.h>

#include "file.hpp"
#include "scope.hpp"

namespace Core {

  nlohmann::json readJsonFile(const std::string& fileName) {
    std::string content;
    std::string line;

    std::ifstream file(fileName);

    assert(file.is_open());

    while(std::getline(file, line)) {
        content += line;
    }

    return nlohmann::json::parse(content);
  }

  void writeJsonFile(const std::string& fileName, const nlohmann::json& json) {
    std::ofstream file(fileName);

    assert(file.is_open());

    file << json;
  }

  bool readSourceFile(const std::string& filename, File& file) {
    LOG(INFO) << "Loading Source File: " << filename;
    
    std::ifstream filestream(filename);
    if(!filestream.is_open()) {
      
      file.filename = filename;
      std::string line;
      while(std::getline(filestream, line)) {
        file.lines.push_back(line);
      }
      
      LOG(DEBUG) << "Read a total of " << file.lines.size() << " lines of code";
    } else {
      LOG(ERROR) << "Could not open file: " << filename;
      return false;
    }
    
    return true;
  }
  
  Scope parseSourceFile(const File& file) {
    Scope scope(ScopeType::Namespace);
    
    return scope;
  }
  
}
