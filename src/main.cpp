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


#include <iostream>
#include <muflihun/easylogging++.h>

#include "core/utils.hpp"
#include "core/file.hpp"
#include "syntax/rule.hpp"


INITIALIZE_EASYLOGGINGPP

#include <regex>
#include <chrono>
//TODO move to desired file
inline bool parseFile(Core::File& file, Core::Scope &outScope)
{
  Core::Scope rootScope(Core::ScopeType::Source);
  // We are 1-indexed
  int lineNo = 1;
  int charNo = 1;
  bool isStillInDefine = false;
  Core::Scope scope;
  for(auto&& line : file.lines)
  {
    // Avoid the costly regex if possible
    if(line.find("#define") != std::string::npos)
    {
      // Should match "     #define" and "   /* some comment */   #define"
      std::regex defineRegex("^(\\s*|\\/\\*.*\\*\\/\\s*)#define");
      std::smatch sm;
      std::regex_search(line, sm, defineRegex);
      if(sm.size() > 0)
      {
        scope = Core::Scope(Core::ScopeType::Namespace);
        scope.lineNumber = lineNo;
        scope.characterNumberStart = sm.position(0)+charNo;
        scope.file = &file;
        
        // Multiline define TODO: Verify with standard
        if(line.at(line.size()-1) == '\\')
        {
          isStillInDefine = true;
          scope.isMultiLine = true;
        }
        else
        {
          scope.characterNumberEnd = scope.characterNumberStart + line.size();
          rootScope.children.push_back(scope);
        }
      }
      else if(isStillInDefine)
      {
        if(line.find("\\") == std::string::npos)
        {
          scope.characterNumberEnd = charNo + line.size()-1;
          rootScope.children.push_back(scope);
          isStillInDefine = false;
        }
      }
    }
    lineNo++;
    charNo+=line.size()+1;
  }
  
  outScope = rootScope;
  
  return true;
}

int main(int argc, char* argv[]) {
  START_EASYLOGGINGPP(argc, argv);
  
  // This would be 10x better in a log.conf
  el::Configurations conf;
  conf.setToDefault();
  conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
  conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
  conf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
  el::Loggers::reconfigureLogger("default", conf);
  
//   auto rules = Syntax::readRules("samples/rules/rules.json");
// 
//   for(const auto& rule : rules) {
//     LOG(INFO) << rule;
//   }
  
  std::map<std::string, Core::File> files;
  std::map<std::string, Core::Scope> parsed_files;
  
  // Recursively get all files
  std::vector<Core::FilesystemItem> stack, current;
  stack = Core::getFilenamesInDirectory("samples");
//   stack = Core::getFilenamesInDirectory("../src"); // Our own source
  
  std::chrono::time_point<std::chrono::system_clock> before = std::chrono::system_clock::now();
  
  while(stack.size() > 0)
  {
    current = stack;
    stack.clear();
    for(auto&& item : current)
    {
      LOG(INFO) << item.fullPath;
      if(item.isDirectory)
      {
        auto temp = Core::getFilenamesInDirectory(item.fullPath);
        stack.insert(stack.begin(), temp.begin(), temp.end());
      }
      else
      {
        Core::File file;
        bool success = Core::readSourceFile(item.fullPath, file);
        if(!success)
        {
          LOG(ERROR) << "Could not read source file '" << item.fullPath << "'";
          continue;
        }
        
        files[item.fullPath] = file;
      }
    }
  }
  
  // Parse all files found
  for(auto&& filePair : files)
  {
    Core::Scope scope;
    bool success = parseFile(filePair.second, scope);
    if(success)
    {
      parsed_files[filePair.first] = scope;
    }
    else
    {
      LOG(ERROR) << "Could not parse source file '" << filePair.first << "'";
    }
  }
  
  // Get namespace scopes
  int total_defines = 0;
  for(auto&& filePair : parsed_files)
  {
    Core::Scope rootScope = filePair.second;
    std::vector<std::string> defineMessages;
    for(auto&& defScope : rootScope.getAllChildrenOfType(Core::ScopeType::Namespace))
    {
//       if(defScope.isMultiLine)
      {
        for(auto&& line : defScope.getScopeLines())
        {
          defineMessages.push_back(line);
        }
      }
      total_defines++;
    }
    
    if(defineMessages.size() > 0)
    {
      LOG(INFO) << "====";
      LOG(INFO) << "Namespace scopes for file " << filePair.first << ":";
      for(auto&& mess : defineMessages)
      {
        LOG(INFO) << mess;
      }
    }
  }
  
  // You can verify with grep -R "#define" samples | wc -l
  LOG(INFO) << "Found " << total_defines << " defines in " << parsed_files.size() << " files";
 
  std::chrono::time_point<std::chrono::system_clock> after = std::chrono::system_clock::now();
  LOG(INFO) << "Ran in " << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count() << "ms";
  
  std::cin.get();
  return 0;
}
