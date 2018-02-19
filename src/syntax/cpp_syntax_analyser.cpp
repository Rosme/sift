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
    REGISTER_RULE(Unknown);
    REGISTER_RULE(NoAuto);
    REGISTER_RULE(NoDefine);
    REGISTER_RULE(NoMacroFunctions);
    REGISTER_RULE(StartWithX);
    REGISTER_RULE(EndWithX);
    REGISTER_RULE(MaxCharactersPerLine);
    REGISTER_RULE(CurlyBracketsOpenSameLine);
    REGISTER_RULE(CurlyBracketsOpenSeperateLine);
    REGISTER_RULE(CurlyBracketsCloseSameLine);
    REGISTER_RULE(CurlyBracketsCloseSeperateLine);
    REGISTER_RULE(AlwaysHaveCurlyBrackets);
    REGISTER_RULE(NoConstCast);
    REGISTER_RULE(StartWithLowerCase);
    REGISTER_RULE(StartWithUpperCase);
    REGISTER_RULE(NameMaxCharacter);
    
    for(const auto& type : RuleType_list)
    {
      const std::string typeString = to_string(type);
      PFE_ASSERT(work.find(type) != work.end(), std::string("Rule '" + typeString + "' is defined but has no work registered for it"));
    }
  }

  std::string CPPSyntaxAnalyser::getRuleMessage(const Syntax::Rule& rule){
    // %rp: rule parameter, %rn: rule name, %rs: rule scope
    std::string ruleMessage = "%rs";

    switch(rule.getRuleType())
    {
      case Syntax::RuleType::StartWithX: ruleMessage = "Expect scopes of type '%rs' to begin with '%rp'"; break;
      case Syntax::RuleType::EndWithX: ruleMessage = "Expect scopes of type '%rs' to end with '%rp'"; break;
      default: break;
    }
        
    return ruleMessage;
  }
  
  void CPPSyntaxAnalyser::RuleUnknown(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack) {
    messageStack.pushMessage(rule.getRuleId(), Core::Message(Core::MessageType::Warning, "Unknown Rule being executed"));
  }

  void CPPSyntaxAnalyser::RuleNoAuto(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    
  } 
  
  void CPPSyntaxAnalyser::RuleNoDefine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    for(auto&& currentScope : rootScope.getAllChildrenOfType(Core::ScopeType::GlobalDefine)) {
      std::stringstream defineLines;

      const auto& lines = currentScope.getScopeLines();
      for(int i = 0; i < lines.size(); i++) {
        defineLines << lines[i];
        if(i < lines.size()-1){
//           defineLines << "\n";          
        }
      }
      
      Core::Message message(Core::MessageType::Error, 
        defineLines.str(), currentScope.lineNumberStart, currentScope.characterNumberStart
      );
      messageStack.pushMessage(rule.getRuleId(), message);
    }
  }
  

  void CPPSyntaxAnalyser::RuleNoMacroFunctions(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    for(auto&& currentScope : rootScope.getAllChildrenOfType(Core::ScopeType::GlobalDefine)) {
      std::string macro;
      std::regex macroSearch(R"(.*#define\s*\w*\(.*)");
      for(const auto& line : currentScope.getScopeLines()) {
        std::smatch match;
        if(std::regex_match(line, match, macroSearch))
        {
          macro = line;
          break;
        }
      }

      if(macro.empty()){
        continue;
      }
      
      Core::Message message(Core::MessageType::Error, 
        macro, currentScope.lineNumberStart, currentScope.characterNumberStart
      );
      messageStack.pushMessage(rule.getRuleId(), message);
    }
  }
  
  
  void CPPSyntaxAnalyser::RuleStartWithX(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Enum | Core::ScopeType::Variable;
    if(rule.getScopeType() != Core::ScopeType::All)
    {
      scopeTypes = rule.getScopeType();
    }
    
    for(auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      const auto& param = rule.getParameter();
      if(currentScope.name.compare(0, param.length(), param) != 0) {
        Core::Message message(Core::MessageType::Error, 
          currentScope.name, currentScope.lineNumberStart, currentScope.characterNumberStart
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }
  
  void CPPSyntaxAnalyser::RuleEndWithX(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Enum | Core::ScopeType::Variable;
    if(rule.getScopeType() != Core::ScopeType::All)
    {
      scopeTypes = rule.getScopeType();
    }
    
    for(auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      const auto& param = rule.getParameter();
      if(currentScope.name.length() < param.length() || currentScope.name.compare(currentScope.name.length()-param.length(), currentScope.name.length(), param) != 0) {
        Core::Message message(Core::MessageType::Error, 
          currentScope.name, currentScope.lineNumberStart, currentScope.characterNumberStart
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }

  void CPPSyntaxAnalyser::RuleMaxCharactersPerLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack) {
    const auto& lines = rootScope.file->lines;
    const auto maxCharPerLine = std::stoul(rule.getParameter());
    for(unsigned int i = 0; i < lines.size(); ++i) {
      if(lines[i].size() > maxCharPerLine) {
        messageStack.pushMessage(rule.getRuleId(), Core::Message(Core::MessageType::Error, 
        SSTR(rule.getParameter() << " expected - got: " << lines[i].size()),
        i, lines[i].size()
      ));
      }
    }
  }

  void CPPSyntaxAnalyser::RuleCurlyBracketsOpenSameLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Conditionnal;
    if (rule.getScopeType() != Core::ScopeType::All) {
      scopeTypes = rule.getScopeType();
    }

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      if (IsScopeUsingCurlyBrackets(currentScope) && IsOpeningCurlyBracketSeparateLine(currentScope)) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name, currentScope.lineNumberStart
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }

  void CPPSyntaxAnalyser::RuleCurlyBracketsOpenSeperateLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Conditionnal;
    if (rule.getScopeType() != Core::ScopeType::All)
    {
      scopeTypes = rule.getScopeType();
    }

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      if (IsScopeUsingCurlyBrackets(currentScope) && !IsOpeningCurlyBracketSeparateLine(currentScope)) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name, currentScope.lineNumberStart
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }



  void CPPSyntaxAnalyser::RuleCurlyBracketsCloseSameLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Conditionnal;
    if (rule.getScopeType() != Core::ScopeType::All)
    {
      scopeTypes = rule.getScopeType();
    }

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      if (IsScopeUsingCurlyBrackets(currentScope) && IsClosingCurlyBracketSeparateLine(currentScope)) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name, currentScope.lineNumberEnd
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }

    }
  }

  void CPPSyntaxAnalyser::RuleCurlyBracketsCloseSeperateLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Conditionnal;
    if (rule.getScopeType() != Core::ScopeType::All)
    {
      scopeTypes = rule.getScopeType();
    }

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      if (IsScopeUsingCurlyBrackets(currentScope) && !IsClosingCurlyBracketSeparateLine(currentScope)) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name, currentScope.lineNumberEnd
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }

  void CPPSyntaxAnalyser::RuleAlwaysHaveCurlyBrackets(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Conditionnal;

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      if (!IsScopeUsingCurlyBrackets(currentScope)) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name, currentScope.lineNumberEnd
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }

  void CPPSyntaxAnalyser::RuleNoConstCast(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack) {
    static auto pushErrorMessage = [&messageStack, &rule](const std::string& line, const Core::Scope& scope) {
      const unsigned int errorOffset = 10;
      int offset = (scope.characterNumberStart <= errorOffset) ? 0 : scope.characterNumberStart - errorOffset;
      Core::Message message(Core::MessageType::Error,
        line.substr(offset, scope.characterNumberEnd - offset + errorOffset),
        scope.lineNumberStart,
        scope.characterNumberStart
      );
      messageStack.pushMessage(rule.getRuleId(), message);
    };

    auto comments = rootScope.getAllChildrenOfType(Core::ScopeType::Comment);
    std::regex constCastRegex(R"(const_cast<.*>\(.*\))");
    const auto& lines = rootScope.file->lines;
    for(unsigned int i = 0; i < lines.size(); ++i) {
      const auto& line = lines[i];
      std::smatch match;
      if(std::regex_search(line, match, constCastRegex)) {
        Core::Scope dummy;
        dummy.lineNumberStart = i;
        dummy.lineNumberEnd = i;
        dummy.characterNumberStart = line.find(match[0]);
        dummy.characterNumberEnd = dummy.characterNumberStart+match[0].str().size()-1;

        if(comments.size() > 0) {
          for(const auto& comment : comments) {
            if(!dummy.isWithinOtherScope(comment)) {
              pushErrorMessage(line, dummy);

            }
          }
        } else {
          pushErrorMessage(line, dummy);
        }
      }
    }
  }

  void CPPSyntaxAnalyser::RuleStartWithLowerCase(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Enum | Core::ScopeType::Variable;
    if (rule.getScopeType() != Core::ScopeType::All)
    {
      scopeTypes = rule.getScopeType();
    }

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      const auto& param = rule.getParameter();
      if (!islower(currentScope.name[0])) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name, currentScope.lineNumberStart, currentScope.characterNumberStart
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }

  void CPPSyntaxAnalyser::RuleStartWithUpperCase(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack)
  {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Enum | Core::ScopeType::Variable;
    if (rule.getScopeType() != Core::ScopeType::All)
    {
      scopeTypes = rule.getScopeType();
    }

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      const auto& param = rule.getParameter();
      if (!isupper(currentScope.name[0])) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name, currentScope.lineNumberStart, currentScope.characterNumberStart
        );
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }

  void CPPSyntaxAnalyser::RuleNameMaxCharacter(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack) {
    Core::ScopeType scopeTypes = Core::ScopeType::Namespace | Core::ScopeType::Class | Core::ScopeType::Function | Core::ScopeType::Enum | Core::ScopeType::Variable;
    if(rule.getScopeType() != Core::ScopeType::All) {
      scopeTypes = rule.getScopeType();
    }

    const auto maxCharPerName = std::stoul(rule.getParameter());
    for(const auto& scope : rootScope.getAllChildrenOfType(scopeTypes)) {
      if(scope.name.size() > maxCharPerName) {
        Core::Message message(Core::MessageType::Error,
                              SSTR(scope.name << " - Got: " << scope.name.size()),
                              scope.lineNumberStart,
                              scope.characterNumberStart);
        messageStack.pushMessage(rule.getRuleId(), message);
      }
    }
  }

  bool CPPSyntaxAnalyser::IsScopeUsingCurlyBrackets(Core::Scope& scope) {
    const std::string& scopeLine = scope.file->lines[scope.lineNumberEnd];
    return scopeLine[scope.characterNumberEnd] == '}';
  }

  bool CPPSyntaxAnalyser::IsOpeningCurlyBracketSeparateLine(Core::Scope& scope) {
    const std::string& scopeLine = scope.file->lines[scope.lineNumberStart];
    for (unsigned int pos = scope.characterNumberStart; pos < scopeLine.size(); ++pos) {
      const char& c = scopeLine[pos];
      if (c == '{') {
        return false;
      }
    }
    return true;
  }

  bool CPPSyntaxAnalyser::IsClosingCurlyBracketSeparateLine(Core::Scope& scope) {
    const std::string& scopeLine = scope.file->lines[scope.lineNumberEnd];
    for (unsigned int pos = 0; pos < scope.characterNumberEnd; ++pos) {
      const char& c = scopeLine[pos];
      if (c != ' ' && c != '\r' && c != '\n') {
        return false;
      }
    }
    return true;
  }

};
