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

#include "config.hpp"
#include "assert.hpp"
#include "file.hpp"

#ifdef UNIX
#include <dirent.h>
#else
#include <Windows.h>
#endif

// Real convenient for debuging
#define DUMP(x) std::cout << #x << ": " << x << std::endl
#define CSTR(x) dynamic_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x ) ).str().c_str()
#define SSTR(x) reinterpret_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x ) ).str()
//

namespace Core {

  inline bool string_ends_with(std::string const& value, std::string const& ending)
  {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }

  inline std::string toLower(const std::string& str) {
    std::string s;

    for(const auto& c : str) {
      s.push_back(tolower(c));
    }

    return s;
  }
  
  inline std::vector<std::string> split(const std::string s, char delim) {
    std::vector<std::string> to_return;
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      to_return.push_back(item);
    }
    
    if(to_return.size() == 0)
      to_return.push_back(s);
    
    return to_return;
  }

  inline bool string_case_compare(const std::string& lhs, const std::string& rhs) {
    return toLower(lhs) == toLower(rhs);
  }

  inline nlohmann::json readJsonFile(const std::string& fileName) {
    std::string content;
    std::string line;

    std::ifstream file(fileName);

    SIFT_ASSERT(file.is_open(), "Could not open Json file for reading");

    while(std::getline(file, line)) {
        content += line;
    }

    return nlohmann::json::parse(content);
  }

  inline void writeJsonFile(const std::string& fileName, const nlohmann::json& json) {
    std::ofstream file(fileName);

    SIFT_ASSERT(file.is_open(), "Could not open Json file for writing");

    file << json;
  }

  inline bool readSourceFile(const std::string& filename, File& file) {
    LOG(TRACE) << "Loading Source File: " << filename;

    std::ifstream filestream(filename);
    if(filestream.is_open()) {

      file.filename = filename;
      std::string line;
      while(std::getline(filestream, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
          line.erase(line.size() - 1);
        file.lines.push_back(line);
      }

      LOG(DEBUG) << "Read a total of " << file.lines.size() << " lines of code";
    } else {
      return false;
    }

    return true;
  }
  
  inline std::string readRawContentSourceFile(const std::string& filename) {
    std::string raw;

    File file;
    if(readSourceFile(filename, file)) {
      for(const auto& line : file.lines) {
        raw += line;
      }
    }

    return raw;
  }

  inline bool directoryExists(const std::string &directory)
  {
    if(!directory.empty())
    {
      #if defined(UNIX)
      DIR* pDirectory;
      if ((pDirectory = opendir(directory.c_str())) != nullptr) {
        closedir(pDirectory);
        return true;
      }
      #elif defined(WIN32)
      DWORD pDirectory = GetFileAttributesA(directory.c_str());
      if(pDirectory & FILE_ATTRIBUTE_DIRECTORY && pDirectory != INVALID_FILE_ATTRIBUTES){
        return true;
      }
      #else
      #error "Current platform not supported"
      #endif
    }

    return false;
  }

  struct FilesystemItem
  {
    bool isDirectory = false;
    std::string name = "";
    std::string fullPath = "";
  };

  inline std::vector<FilesystemItem> getFilenamesInDirectory(const std::string directory)
  {
    std::vector<FilesystemItem> toReturn;
    #if defined(UNIX)
    DIR* pDirectory;
    struct dirent *ent;
    if ((pDirectory = opendir(directory.c_str())) != nullptr)
    {
      while ((ent = readdir(pDirectory)) != nullptr)
      {
        // Filter out ../.
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
        {
          FilesystemItem item;
          item.name = ent->d_name;
          item.isDirectory = (ent->d_type == DT_DIR);
          item.fullPath = directory + "/" + item.name;
          toReturn.push_back(item);
        }
      }
      closedir(pDirectory);
    }
    #elif defined(WIN32)
    HANDLE hFind;
    WIN32_FIND_DATA data;

    hFind = FindFirstFile(CSTR(directory.c_str() << "\\*.*"), &data);
    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        // Filter out ../.
        if(strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0)
        {
          FilesystemItem item;
          item.name = data.cFileName;
          item.isDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
          item.fullPath = directory + "/" + item.name;
          toReturn.push_back(item);
        }
      } while (FindNextFile(hFind, &data));
      FindClose(hFind);
    }
    #else
    #error "Current platform not supported"
    #endif

    return toReturn;
  }
}
