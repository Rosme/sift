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

#include <regex>

#include "../core/file.hpp"
#include "../core/message.hpp"

namespace Syntax
{
  CPPSyntaxAnalyser::CPPSyntaxAnalyser()
  {
    
  }
  
  CPPSyntaxAnalyser::~CPPSyntaxAnalyser()
  {
  }
  
  // Force consistency between name and method
  #define NS(ns,item) ns::item
  #define REGISTER_RULE(REG) work[NS(RuleType, REG)] = std::bind(&CPPSyntaxAnalyser::Rule##REG, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
  void CPPSyntaxAnalyser::registerRuleWork(std::map<Syntax::RuleType, std::function<void(Syntax::Rule&, Core::Scope&, Core::MessageStack&)>>& work)
  {
    // Registers a rule, expects a name in Syntax::RuleType::RULENAME and a function named CPPSyntaxAnalyser::RuleRULENAME;
    REGISTER_RULE(NoDefine);
    REGISTER_RULE(NoMacroFunctions);
    REGISTER_RULE(StartWithX);
    REGISTER_RULE(EndWithX);
  }
  
  void CPPSyntaxAnalyser::RuleNoDefine(Syntax::Rule& rule, Core::Scope& scope, Core::MessageStack& messageStack)
  {
    for(auto&& currentScope : scope.getAllChildrenOfType(Core::ScopeType::GlobalDefine)) {
      std::stringstream defineLines;

      for(auto&& line : currentScope.getScopeLines()) {
        defineLines << line << "\n";
      }
      
      Core::Message message(Core::MessageType::Error, 
        SSTR("Define found - " << currentScope.file->filename << " vvvv"
        "\n -->" << defineLines.str()), currentScope.lineNumberStart, currentScope.characterNumberStart
      );
      messageStack.pushMessage(message);
    }
  }
  
 void CPPSyntaxAnalyser::RuleNoMacroFunctions(Syntax::Rule& rule, Core::Scope& scope, Core::MessageStack& messageStack)
  {
    for(auto&& currentScope : scope.getAllChildrenOfType(Core::ScopeType::GlobalDefine)) {
      std::string macro;
      std::regex macroSearch(R"(#define\s*\w*\(.*)");
      for(const auto& line : currentScope.getScopeLines()) {
        std::smatch match;
        if(std::regex_match(line, match, macroSearch))
        {
          macro = line + "\n";
          break;
        }
      }

      if(macro.empty()){
        continue;
      }
      
      Core::Message message(Core::MessageType::Error, 
        SSTR("Macro function found - " << currentScope.file->filename << " vvvv"
        "\n -->" << macro), currentScope.lineNumberStart, currentScope.characterNumberStart
      );
      messageStack.pushMessage(message);
    }
  }
  
  
  void CPPSyntaxAnalyser::RuleStartWithX(Syntax::Rule& rule, Core::Scope& scope, Core::MessageStack& messageStack)
  {
    for(auto&& currentScope : scope.getAllChildrenOfType(rule.getScopeType())) {
      const auto& param = rule.getParameter();
      if(currentScope.name.compare(0, param.length(), param) != 0) {
        Core::Message message(Core::MessageType::Error, 
          SSTR("Prefix does not match (want: " << rule.getParameter() << ") - " << currentScope.file->filename << " vvvv"
          "\n -->" << currentScope.name), currentScope.lineNumberStart, currentScope.characterNumberStart
        );
        messageStack.pushMessage(message);
      }
    }
  }
  
  void CPPSyntaxAnalyser::RuleEndWithX(Syntax::Rule& rule, Core::Scope& scope, Core::MessageStack& messageStack)
  {
    for(auto&& currentScope : scope.getAllChildrenOfType(rule.getScopeType())) {
      const auto& param = rule.getParameter();
      if(currentScope.name.length() < param.length() || currentScope.name.compare(currentScope.name.length()-param.length(), currentScope.name.length(), param) != 0) {
        Core::Message message(Core::MessageType::Error, 
          SSTR("Suffix does not match (want: " << rule.getParameter() << ") - " << currentScope.file->filename << " vvvv"
          "\n -->" << currentScope.name), currentScope.lineNumberStart, currentScope.characterNumberStart
        );
        messageStack.pushMessage(message);
      }
    }
  }
};
