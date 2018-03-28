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

#include "cpp_flow_analyser.hpp"

#include <regex>

#include "../core/file.hpp"
#include "../core/message.hpp"

namespace Flow
{
  CPPFlowAnalyser::CPPFlowAnalyser()
  {
    
  }
  
  CPPFlowAnalyser::~CPPFlowAnalyser()
  {
  }

  void CPPFlowAnalyser::analyzeFlow(Core::Scope& rootScope, Core::MessageStack& messageStack) {
    analyzeNullPointer(rootScope, messageStack);
    analyzeUninitializedVariable(rootScope, messageStack);
  }

  void CPPFlowAnalyser::analyzeNullPointer(Core::Scope& rootScope, Core::MessageStack& messageStack) {
    Core::ScopeType scopeTypes = Core::ScopeType::Variable;

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      int nullPointerLine = scopeUsingNullPointer(currentScope);
      if (nullPointerLine > -1) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name + " will throw a NULL pointer exception", nullPointerLine
        );
        messageStack.pushMessage(1, message);
      }
    }
  }

  void CPPFlowAnalyser::analyzeUninitializedVariable(Core::Scope& rootScope, Core::MessageStack& messageStack) {
    Core::ScopeType scopeTypes = Core::ScopeType::Variable;

    for (auto&& currentScope : rootScope.getAllChildrenOfType(scopeTypes)) {
      int uninitializedVariableLine = scopeUsingUninitializedVariable(currentScope);
      if (uninitializedVariableLine > -1) {
        Core::Message message(Core::MessageType::Error,
          currentScope.name + " is used before initialization", uninitializedVariableLine
        );
        messageStack.pushMessage(2, message);
      }
    }
  }

  int CPPFlowAnalyser::scopeUsingNullPointer(Core::Scope& scope) {
    //Regex = \s+\*(VARNAME)(\s+|=|\(|{|;)
    std::regex isVariablePointerRegex(R"(\s+\*()" + scope.name + R"()(\s+|=|\(|\{|;))");
    const std::string& lineVariableDeclaration = scope.file->lines[scope.lineNumberStart];
    std::smatch match;
    std::regex_search(lineVariableDeclaration, match, isVariablePointerRegex);
    if (match.size() > 0) {
      Core::Scope* parentScope = scope.parent;
      
      if (parentScope->isOfType(Core::ScopeType::Function) || parentScope->isOfType(Core::ScopeType::Conditional)) {
        std::string varValue;

        std::string regexValue = match[0];
        int positionAfterVarName = lineVariableDeclaration.find(regexValue);
        int regexLength = regexValue.length();
        positionAfterVarName += regexLength - 1;

        isVariableValueChanged(lineVariableDeclaration, true, positionAfterVarName, varValue);

        std::regex variableRegex(R"((^|\s)()" + scope.name + R"()(\s+|=|\(|\{|;))");
        for (unsigned int i = scope.lineNumberStart + 1; i < parentScope->lineNumberEnd; ++i) {
          const std::string line = parentScope->file->lines[i];
          std::regex_search(line, match, variableRegex);
          if (match.size() > 0) {
            std::string regexValue = match[0];
            positionAfterVarName =  line.find(regexValue);
            regexLength = regexValue.length();
            positionAfterVarName += regexLength - 1;

            if (!isVariableValueChanged(line, false, positionAfterVarName, varValue)) {
              if (!isVariableValueValid(varValue, true)) {
                return i;
              }
            }
          }
        }
      }

    }
    return -1;
  }

  int CPPFlowAnalyser::scopeUsingUninitializedVariable(Core::Scope& scope) {
    Core::Scope* parentScope = scope.parent;
    if (isScopeVariablePrimitive(scope) && (parentScope->isOfType(Core::ScopeType::Function) || parentScope->isOfType(Core::ScopeType::Conditional))) {
      std::string varValue;
      std::regex variableRegex(R"((^|\s)()" + scope.name + R"()(\s+|=|\(|\{|;))");
      for (unsigned int i = scope.lineNumberStart; i < parentScope->lineNumberEnd; ++i) {
       
        const std::string line = parentScope->file->lines[i];
        std::smatch match;
        std::regex_search(line, match, variableRegex);
        if (match.size() > 0) {
          std::string regexValue = match[0];
          int positionAfterVarName = line.find(regexValue);
          int regexLength = regexValue.length();
          positionAfterVarName += regexLength - 1;

          if (!isVariableValueChanged(line, i == scope.lineNumberStart, positionAfterVarName, varValue)) {
            if (varValue.empty() && i != scope.lineNumberStart) {
              return i;
            }
          }
        }
      }
    }
    return -1;
  }

  bool CPPFlowAnalyser::isScopeVariablePrimitive(Core::Scope& scope) {
    if (scope.isOfType(Core::ScopeType::Variable)) {
      const std::string line = scope.file->lines[scope.lineNumberStart];
      std::regex variableRegex(R"((^|\s)(bool|int|char|double|float|long|short) )" + scope.name + R"((\s+|=|\(|\{|;))");
      std::smatch match;
      std::regex_search(line, match, variableRegex);
      return match.size() > 0;
    }
    return false;
  }

  //positionAfterVarName = the position on line after the variable name appear (if var name end at position 10, use 11 for this variable)
  bool CPPFlowAnalyser::isVariableValueChanged(const std::string& line, bool isVariableDeclarationLine, int positionAfterVarName, std::string& varValue) {
    bool isInitializingVariable = false;
    int counter = 0;
    bool isUsingParenthesis = false;
    bool isUsingBracket = false;
    int initializationStartingCharacter = 0;
    for (int i = positionAfterVarName; i < line.length(); ++i) {
      const char& c = line[i];
      if (!isspace(c)) {
        // Check if the variable value is changed on the line
        // return if value is not changed
        if (!isInitializingVariable) {
          if (c == '=')  {
            if (line[i + 1] == '=') {
              return false;
            }
            isInitializingVariable = true;
            initializationStartingCharacter = i + 1;
          } else if (c == '{' && isVariableDeclarationLine) {
            isInitializingVariable = true;
            isUsingBracket = true;
            counter = 1;
            initializationStartingCharacter = i + 1;
          } else if (c == '(' && isVariableDeclarationLine) {
            isInitializingVariable = true;
            isUsingParenthesis = true;
            counter = 1;
            initializationStartingCharacter = i + 1;
          } else {
            return false;
          }
        } else {

          if (c == '{' && !isUsingParenthesis) {
            if (counter == 0) {
              initializationStartingCharacter = i + 1;
              isUsingBracket = true;
            }
            counter++;
          } else if (c == '}' && !isUsingParenthesis) {
            counter--;
            if (counter == 0) {
              varValue = line.substr(initializationStartingCharacter, i - initializationStartingCharacter);
              return true;
            }
          } else if (c == '(' && !isUsingBracket) {
            if (counter == 0) {
              initializationStartingCharacter = i + 1;
              isUsingParenthesis = true;
            }
            counter++;
          } else if (c == ')' && !isUsingBracket) {
            counter--;
            if (counter == 0) {
              varValue = line.substr(initializationStartingCharacter, i - initializationStartingCharacter);
              return true;
            }
          } else if (!isUsingBracket && !isUsingParenthesis){
            varValue = line.substr(initializationStartingCharacter, line.find(';') - initializationStartingCharacter);
            return true;
          }

        }
      }
    }
    return false;
  }

  bool CPPFlowAnalyser::isVariableValueValid(std::string varValue, bool isPointer) {
    if (varValue.empty()) {
      return false;
    }
    //Trim the value
    const std::string& listEmptyCharacters = "\t\n\v\f\r ";
    varValue.erase(0, varValue.find_first_not_of(listEmptyCharacters));
    varValue.erase(varValue.find_last_not_of(listEmptyCharacters) + 1);

    if (varValue == "NULL" || (varValue == "0" && isPointer)) { 
      return false;
    }

    return true;
  }
};
