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


//TODO move to desired file
inline bool parseFile(const Core::File& file, Core::Scope &outScope)
{
  Core::Scope rootScope(Core::ScopeType::Source);
  int lineNo = 0;
  for(auto&& line : file.lines)
  {
    if(line.find("#define") != std::string::npos)
    {
      Core::Scope define(Core::ScopeType::Namespace);
      define.setLineNumber(lineNo);
      
    }
    lineNo++;
  }
  
  outScope = rootScope;
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
  
  std::vector<Core::FilesystemItem> stack, current;
  stack = Core::getFilenamesInDirectory("samples");
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
 
  std::cin.get();
  return 0;
}
