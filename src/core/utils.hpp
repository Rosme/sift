/* MIT License
 *
 * Copyright (c) 2018 Jean-Sebastien Fauteux, Michel Rioux, Raphaël Massabot
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <string>
#include <fstream>
#include <cassert>

#include <nlohmann/json.hpp>
#include <muflihun/easylogging++.h>

#include "assert.hpp"
#include "file.hpp"

namespace Core {

  inline nlohmann::json readJsonFile(const std::string& fileName) {
    std::string content;
    std::string line;

    std::ifstream file(fileName);

    PFE_ASSERT(file.is_open(), "Could not open Json file for reading");

    while(std::getline(file, line)) {
        content += line;
    }

    return nlohmann::json::parse(content);
  }

  inline void writeJsonFile(const std::string& fileName, const nlohmann::json& json) {
    std::ofstream file(fileName);

    PFE_ASSERT(file.is_open(), "Could not open Json file for writing");

    file << json;
  }

  inline bool readSourceFile(const std::string& filename, File& file) {
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
  
  inline std::string toLower(const std::string& str) {
    std::string s;

    for(const auto& c : str) {
      s.push_back(tolower(c));
    }

    return s;
  }

  inline bool string_case_compare(const std::string& lhs, const std::string& rhs) {
    return toLower(lhs) == toLower(rhs);
  }

}
