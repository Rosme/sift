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

#include "cpp_syntax_analyser.hpp"

#include "../core/file.hpp"

namespace Syntax
{
  CPPSyntaxAnalyser::CPPSyntaxAnalyser()
  {
    
  }
  
  CPPSyntaxAnalyser::~CPPSyntaxAnalyser()
  {
  }
  
  void CPPSyntaxAnalyser::RuleNoDefine(Syntax::Rule& rule, Core::Scope& scope, Core::MessageStack& messageStack)
  {
    // Get namespace scopes
    int total_defines = 0;
    int definesInFile = 0;
    Core::Scope rootScope = scope;
    std::vector<std::string> defineMessages;
    Core::File* current = nullptr;
    for(auto&& defScope : rootScope.getAllChildrenOfType(Core::ScopeType::Namespace))
    {
      {
        if(current != defScope.file)
        {
          LOG(INFO) << "Found " << definesInFile << " define(s) in this file";
          LOG(INFO) << "====";
          LOG(INFO) << "Namespace scopes for file " << defScope.file->filename;
          current = defScope.file;
          definesInFile = 0;
        }
        int i =0;
        for(auto&& line : defScope.getScopeLines())
        {
          defineMessages.push_back(line);
          LOG(INFO) << "  " << (i++ == 0 ? "->" : "  ") << line;
        }
      }
      total_defines++;
      definesInFile++;
    }
    
    // You can verify with grep -R "#define" samples | wc -l
//     LOG(INFO) << "Found " << total_defines << " defines in " << pfe.getScopes().size() << " files";
  }
};
