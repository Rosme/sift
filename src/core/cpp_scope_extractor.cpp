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

#include <regex>
#include <stack>
#include <muflihun/easylogging++.h>

#include "constants.hpp"
#include "cpp_scope_extractor.hpp"
#include "scope.hpp"

namespace Core {

  //http://en.cppreference.com/w/cpp/keyword
  const std::vector<std::string> CppScopeExtractor::ReservedKeywords({
    "if", 
    "else", 
    "else if", 
    "while", 
    "do", 
    "new", 
    "delete", 
    "typedef",
    "try",
    "catch",
    "return",
    "public",
    "static",
    "typedef"
  });


  bool CppScopeExtractor::extractScopesFromFile(File& file, Scope &outScope) {
    Scope rootScope(ScopeType::Source);
    rootScope.file = &file;
    rootScope.name = file.filename;
    rootScope.lineNumberStart = 0;
    rootScope.lineNumberEnd = file.lines.size();
    rootScope.characterNumberStart = 0;
    rootScope.characterNumberEnd = 0;

    try{
      extractComments(file, rootScope); //This is needed for later use
      extractStringLiterals(file, rootScope);

      //The idea here is to fill the rootscope and have all the scopes on a flat line at no depth
      //Afterward, we will construct the tree correctly based on analysis of which scope is within whom
      //This allows us to not have much recursion and handle pretty much all edge case of scope within scopes.
      extractDefines(file, rootScope);
      
      m_defineScopes[file.filename] = rootScope.getAllChildrenOfType(Core::ScopeType::GlobalDefine);
      
      extractNamespaces(file, rootScope);
      extractEnums(file, rootScope);
      extractClasses(file, rootScope);
      extractFunctions(file, rootScope);
      extractVariables(file, rootScope);
      extractConditionals(file, rootScope);

      //Filtering out reserved keywords
      rootScope.children.erase(std::remove_if(rootScope.children.begin(), rootScope.children.end(), [](const Scope& scope) {
        for(const auto& keyword : ReservedKeywords) {
          if(scope.name == keyword && scope.type != ScopeType::Conditional) {
            return true;
          }
        }
        return false;
      }), rootScope.children.end());

      //Remove scope within comments
      filterScopes(rootScope);

      LOG(TRACE) << "\n" << rootScope.getTree();

      outScope = rootScope;
    }catch(std::overflow_error& e){
      LOG(ERROR) << e.what();
      return false;
    }
      
    return true;
  }

  void CppScopeExtractor::extractDefines(File& file, Scope& parent) {
    int lineNo = 0;
    bool isStillInDefine = false;
    Scope scope;
    for(auto&& line : file.lines) {
      if(line.find("#define") != std::string::npos) {
        // Should match "     #define" and "   /* some comment */   #define"
        std::regex defineRegex(R"(^(\s*|\s*\/\*.*\*\/\s*)(#define))");
        std::smatch sm;
        std::regex_search(line, sm, defineRegex);
        if(sm.size() > 0) {
          scope = Scope(ScopeType::GlobalDefine);
          scope.lineNumberStart = lineNo;
          scope.characterNumberStart = sm.position(2);
          scope.file = &file;

          if(line.at(line.size()-1) == '\\') {
            isStillInDefine = true;
          } else {
            size_t pos = -1; 
            if((pos = line.find("/*")) != std::string::npos || (pos = line.find("//")) != std::string::npos){
              // /* something like this */ #define
              if(pos < scope.characterNumberStart){
                scope.characterNumberEnd = scope.characterNumberStart + line.size();
              }else{
                scope.characterNumberEnd = pos-2;
              }
            }else{
              scope.characterNumberEnd = scope.characterNumberStart + line.size();
            }
            
            scope.lineNumberEnd = lineNo;
            scope.name = line;

            parent.children.push_back(scope);
          }
        }
      } else if(isStillInDefine) {
        if(line.find("\\") == std::string::npos) {
          scope.characterNumberEnd = line.size();
          scope.lineNumberEnd = lineNo;
          scope.name = scope.getScopeLines().at(0);
          parent.children.push_back(scope);
          isStillInDefine = false;
        }
      }
      lineNo++;
    }

  }

  void CppScopeExtractor::extractNamespaces(File& file, Scope& parent) {
    // Regex that match namespace without using in front
    // The regex supports space or no space after the name, and any kind of return line (UNIX/Windows)
    std::regex namespaceRegex(R"(^(?!using)\s*namespace (\w*)(\n|\r\n)*\s*(\{*))");
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, namespaceRegex);
      if(match.size() > 0) {
        Scope scope(ScopeType::Namespace);
        scope.name = match[1];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.file = &file;

        //Namespace could have a namespace
        //We still need to parse every line, can't skip to end of namespace
        findEndOfScope(scope, file, lineNumber);

        LOG(DEBUG) << "\n" << scope;

        /*extractNamespaces(file, scope);
        extractEnums(file, scope);*/

        //Adding to parent the scope
        parent.children.push_back(scope);
      }
    }
  }

  void CppScopeExtractor::extractEnums(File& file, Scope& parent) {
    std::regex enumRegex(R"(\s*enum(\s*class)?\s*(\w*)\s*:?\s*(\w|\s)*\s*\{?)");
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, enumRegex);

      if(match.size() > 0) {
        Scope scope(ScopeType::Enum);
        scope.name = match[2];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.file = &file;

        //An enum can't contain another enum, can skip directly to the end of it
        lineNumber = findEndOfScope(scope, file, lineNumber);

        LOG(DEBUG) << "\n" << scope;

        parent.children.push_back(scope);
      }
    }
  }

  void CppScopeExtractor::extractClasses(File& file, Scope& parent) {
    std::regex classRegex(R"((?!enum)\s*(class|struct|union)\s*(\w*)\s*:?\s*(\w|\s)*\s*\{?)");
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, classRegex);
      if(match.size() > 0) {
        Scope scope(ScopeType::Class);
        scope.name = match[2];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[1]);
        scope.file = &file;

        //Struct/Class can have Struct/Class inside
        findEndOfScope(scope, file, lineNumber);

        LOG(DEBUG) << "\n" << scope;

        parent.children.push_back(scope);
      }
    }
  }

  void CppScopeExtractor::extractFunctions(File& file, Scope& parent) {
    std::regex functionRegex(R"(((\w|:)*\s)*((\w|:~?)+)\s*\((.*),?\).*\s*(;|\{)?)");
    
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, functionRegex);
      if(match.size() > 0) {
        if(isLineWithinDefine(file.filename, lineNumber)){
          break;
        }
    
        Scope scope(ScopeType::Function);
        scope.name = match[3];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.file = &file;

        std::size_t semicolonPos = line.find(';', scope.characterNumberStart);
        std::size_t curlyBracketPos = line.find('{', scope.characterNumberStart);

        for(unsigned int j = lineNumber; semicolonPos == std::string::npos && curlyBracketPos == std::string::npos; ++j) {
          if(j >= parent.lineNumberEnd){
            throw std::overflow_error("Reached end of parent scope, something is probably wrong");
          }
          
          const std::string& nextLine = file.lines[j];
          semicolonPos = nextLine.find(';', scope.characterNumberStart);
          curlyBracketPos = nextLine.find('{', scope.characterNumberStart);
        }

        if(semicolonPos != std::string::npos) {
            scope.characterNumberEnd = semicolonPos;
            scope.lineNumberEnd = scope.lineNumberStart;
        } else {
          findEndOfScope(scope, file, lineNumber);
        }

        LOG(DEBUG) << "\n" << scope;

        parent.children.push_back(scope);
      }
    }
    
    
  }

  void CppScopeExtractor::extractVariables(File& file, Scope& parent) {
    std::regex variableRegex(R"(\s*(?!return)(((\w*::)*\w+\*?\s+)|(\blong\b|\bsigned\b|\bunsigned\b\s*(\w*::)*\w+\*?\s+)|((\w*::)*\w+\s+\*?)|((\w*::)*\w+\s+\*\s+))(\w+)(\[\d*\])*\s*((\(.*\))|(=[^!=]\s*.*))?\s*;)");
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, variableRegex);
      if(match.size() > 0) {
        if(isLineWithinDefine(file.filename, lineNumber)){
          break;
        }
        
        Scope scope(ScopeType::Variable);
        scope.name = match[10];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.lineNumberEnd = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.characterNumberEnd = line.find(';', scope.characterNumberStart);
        scope.file = &file;

        LOG(DEBUG) << "\n" << scope;

        //Adding to parent the scope
        parent.children.push_back(scope);
      }
    }
  }

  void CppScopeExtractor::extractConditionals(File& file, Scope& parent) {
    std::regex conditionalRegex(R"((^|\s)(if|else|for|switch|while|do)(\(|\{|\s|$))");
    for(unsigned int i = parent.lineNumberStart; i < parent.lineNumberEnd; ++i) {
      const std::string& line = file.lines[i];
      std::smatch match;
      std::regex_search(line, match, conditionalRegex);
      if(match.size() > 0) {
        if(isLineWithinDefine(file.filename, i) || isWithinStringLiteral(line.find(match[0]), i, file) || isWithinComment(line.find(match[0]), i, file, parent)) {
          continue;
        }
        
        if(match[2] == "while") {
          if(isDoWhileLoop(file, i, line.find(match[0]))) {
            continue;
          }
        }
        Scope scope(ScopeType::Conditional);
        scope.name = match[2];
        scope.parent = &parent;
        scope.lineNumberStart = i;
        scope.characterNumberStart = line.find(match[2]);
        scope.file = &file;

        if(match[2] == "for") {
          findEndOfScopeConditionalFor(scope, file, i, scope.characterNumberStart);
        } else if(match[2] == "do") {
          findEndOfScopeConditionalDoWhile(scope, file, i + 1);
        } else {
          findEndOfScope(scope, file, i, scope.characterNumberStart);
        }

        LOG(DEBUG) << "\n" << scope;

        //Adding to parent the scope
        parent.children.push_back(scope);
      }
    }
  }

  void CppScopeExtractor::extractComments(File & file, Scope & parent) {
    std::regex singleLineComments(R"(.*(\/\/.*))");
    std::regex multiLineComments(R"(.*(\/\*))");
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      if(line.empty()) {
        continue;
      }
      if(std::regex_search(line, match, multiLineComments)) {
        Scope scope(ScopeType::MultiLineComment);
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[1]);
        scope.file = &file;

        std::string name = "";
        for(unsigned int i = lineNumber; i < file.lines.size(); ++i) {
          name += file.lines[i];
          auto endIndex = file.lines[i].find("*/");
          if(endIndex != std::string::npos) {
            scope.characterNumberEnd = endIndex+1; // Want to point to / after the star
            scope.lineNumberEnd = i;
            lineNumber = i;
            break;
          }
        }
        scope.name = name;

        LOG(DEBUG) << "\n" << scope;

        parent.children.push_back(scope);
        m_comments[file.filename].push_back(scope); //TODO maybe remove comments from other scope containers
      } else if(std::regex_match(line, match, singleLineComments)) {
        Scope scope(ScopeType::SingleLineComment);
        scope.name = match[1];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.lineNumberEnd = lineNumber;
        scope.characterNumberStart = line.find(match[1]);
        scope.characterNumberEnd = line.size()-1;
        scope.file = &file;

        LOG(DEBUG) << "\n" << scope;

        parent.children.push_back(scope);
        m_comments[file.filename].push_back(scope);
      }
    }
  }

  void CppScopeExtractor::extractStringLiterals(File& file, Scope& parent) {
    //TODO: This is ugly, fix this
    std::size_t startIndex = 0;
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      startIndex = line.find('"', startIndex+1);
      if(startIndex == std::string::npos) {
        startIndex = 0;
        continue;
      }
      
      if(isWithinComment(startIndex, lineNumber, file, parent)){
        continue;
      }
      
      if(isWithinStringLiteral(startIndex, lineNumber, file)){
        continue;
      }
      
      if(startIndex > 0 && line[startIndex-1] == '\\') {
        --lineNumber;
        continue;
      }

      Scope scope(ScopeType::StringLiteral);
      scope.lineNumberStart = lineNumber;
      scope.lineNumberEnd = lineNumber;
      scope.characterNumberStart = startIndex;

      auto endIndex = line.find('"', startIndex+1);
      while(endIndex != std::string::npos && line[endIndex-1] == '\\' && (endIndex >= 2 && line[endIndex-2] != '\\')) {
        endIndex = line.find('"', endIndex+1);
      }

      if(endIndex == std::string::npos) {
        scope.characterNumberEnd = line.size()-1;
      } else {
        scope.characterNumberEnd = endIndex;
      }

      scope.name = line.substr(startIndex, scope.characterNumberEnd-startIndex);
      scope.file = &file;

      if(scope.characterNumberEnd != line.size()-1) {
        --lineNumber;
        startIndex = scope.characterNumberEnd+1;
      } else {
        startIndex = 0;
      }

      m_stringLiterals[file.filename].push_back(scope);   
    }

    startIndex = 0;
    for(unsigned int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      startIndex = line.find('\'', startIndex);
      if(startIndex == std::string::npos) {
        startIndex = 0;
        continue;
      }
      
      if(isWithinComment(startIndex, lineNumber, file, parent)){
        continue;
      }
      
      if(isWithinStringLiteral(startIndex, lineNumber, file)){
        continue;
      }

      // Handle '\''
      if(startIndex > 0 && line[startIndex-1] == '\\') {
        
        // But ignore '\\'
        if(startIndex < 2 || line[startIndex-2] != '\\'){
          --lineNumber;
          continue;
        }
      }

      Scope scope(ScopeType::StringLiteral);
      scope.lineNumberStart = lineNumber;
      scope.lineNumberEnd = lineNumber;
      scope.characterNumberStart = startIndex;

      //Line ends with it
      if(startIndex == line.size()-1) {
        scope.characterNumberEnd = startIndex;
      } else if(line[startIndex+1] == '\\') {
        scope.characterNumberEnd = startIndex + 3;
      } else {
        scope.characterNumberEnd = startIndex + 2;
      }

      scope.name = line.substr(startIndex, scope.characterNumberEnd-startIndex);
      scope.file = &file;

      if(scope.characterNumberEnd < line.size()-1) {
        --lineNumber;
        startIndex = scope.characterNumberEnd+1;
      } else {
        startIndex = 0;
      }

      m_stringLiterals[file.filename].push_back(scope);
    }
  }

  void CppScopeExtractor::filterScopes(Scope& root) {
    
    auto childrenCopy = root.children;
    root.children.erase(std::remove_if(root.children.begin(), root.children.end(), [&childrenCopy](const Scope& scope) {
      for(auto& it : childrenCopy) {
        if(it == scope) {
          continue;
        }

        //Filtering comments
        if(scope.isWithinOtherScope(it) && it.isOfType(ScopeType::Comment)) {
          return true;
        } else if(scope.isOfType(ScopeType::Function) && scope.isWithinOtherScope(it) && (it.isOfType(ScopeType::Function) || it.isOfType(ScopeType::Conditional))) {
          //Filtering function call within a function
          return true;
        }
        

      }

      return false;
    }), root.children.end());
  }

  void CppScopeExtractor::constructTree(Scope& root) {

    std::sort(root.children.begin(), root.children.end(), [](const Scope& lhs, const Scope& rhs) {
      return lhs.lineNumberStart < rhs.lineNumberStart;
    });

    //Finding the parent of every Scope
    for(auto it = root.children.rbegin(); it != root.children.rend(); it++) {
      if(it->type == ScopeType::GlobalDefine) {
        it->parent = &root;
        continue;
      }

      Scope& bestParent = findBestParent(root, *it);
      it->parent = &bestParent;
      //Checking if it's a variable or function, and being more precise about it if it's the case
      if(it->type == ScopeType::Variable) {
        if(bestParent.type == ScopeType::Source || bestParent.type == ScopeType::Namespace) {
          it->type = ScopeType::GlobalVariable;
        } else if(bestParent.type == ScopeType::Conditional || bestParent.type == ScopeType::Function) {
          it->type = ScopeType::FunctionVariable;
        } else {
          it->type = ScopeType::ClassVariable;
        }
      } else if(it->type == ScopeType::Function) {
        if(bestParent.type == ScopeType::Class) {
          it->type = ScopeType::ClassFunction;
        } else {
          if(it->name.find(':') != std::string::npos) {
            it->type = ScopeType::ClassFunction;
          } else if(bestParent.isOfType(ScopeType::Function)) {
            //Declared like a function(e.g. int var(5))
            it->type = ScopeType::FunctionVariable;
          } else {
            it->type = ScopeType::FreeFunction;
          }
        }
      }
    }
  }

  Scope& CppScopeExtractor::findBestParent(Scope& root, Scope& toSearch) {
    Scope* bestCandidate = &root;

    for(unsigned int i = 0; i < root.children.size(); ++i) {
      Scope& child = root.children[i];
      if(toSearch.isWithinOtherScope(child) && toSearch != child) {
        if(child.isWithinOtherScope(*bestCandidate)) {
          bestCandidate = &child;
        }
      }
    }

    return *bestCandidate;
  }

  bool CppScopeExtractor::isWithinStringLiteral(unsigned int pos, unsigned int line, File& file) const {
    Scope dummy;
    dummy.characterNumberStart = pos;
    dummy.characterNumberEnd = pos;
    dummy.lineNumberStart = line;
    dummy.lineNumberEnd = line;
    dummy.file = &file;

    auto it = m_stringLiterals.find(file.filename);
    if(it != m_stringLiterals.end()) {
      return std::find_if(it->second.begin(), it->second.end(), [&dummy](const Scope& stringScope) {
        return dummy.isWithinOtherScope(stringScope);
      }) != it->second.end();
    }

    return false;
  }

  bool CppScopeExtractor::isWithinComment(unsigned int pos, unsigned int line, File& file, Scope& parent) const {
    Scope dummy;
    dummy.characterNumberStart = pos;
    dummy.characterNumberEnd = pos;
    dummy.lineNumberStart = line;
    dummy.lineNumberEnd = line;
    dummy.file = &file;

    auto it = m_comments.find(file.filename);
    if(it != m_comments.end()) {
      return std::find_if(it->second.begin(), it->second.end(), [&dummy](const Scope& stringScope) {
        return dummy.isWithinOtherScope(stringScope);
      }) != it->second.end();
    }
    

    return false;
  }

  int CppScopeExtractor::findEndOfScope(Scope& scope, File& file, int startingLine) {
   
    std::stack<char> bracketStack;
    int scopeLineNumber = startingLine;
    
    for(unsigned int j = startingLine; j < file.lines.size(); ++j) {
      scopeLineNumber = j;
      const std::string& namespaceLine = file.lines[j];
      for(unsigned int pos = 0; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        if(c == '{') {
          if(!isWithinStringLiteral(pos, scopeLineNumber, file)) {
            bracketStack.push('{');
          }
        } else if(c == '}') {
          if(!isWithinStringLiteral(pos, scopeLineNumber, file)) {
            bracketStack.pop();
            if(bracketStack.size() == 0) {
              scope.characterNumberEnd = pos;
              scope.lineNumberEnd = scopeLineNumber;
              j = file.lines.size();
              break;
            }
          }
        }
        
        if(bracketStack.size() > BRACKET_STACK_GIVEUP){
          throw std::overflow_error("Reached maximum bracket size, something is probably wrong");
        }
      }

    }

    return scopeLineNumber;
  }

  int CppScopeExtractor::findEndOfScope(Scope& scope, File& file, int startingLine, int startingCharacter) {    
    std::stack<char> bracketStack;
    int scopeLineNumber = startingLine;
    int startingCharForThisLine = startingCharacter;

    for(unsigned int j = startingLine; j < file.lines.size(); ++j) {
      scopeLineNumber = j;
      const std::string& namespaceLine = file.lines[j];
      for(unsigned int pos = startingCharForThisLine; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        
        if(c == '{') {
          if(!isWithinStringLiteral(pos, scopeLineNumber, file)) {
            bracketStack.push('{');
          }
        } else if(c == '}') {
          if(!isWithinStringLiteral(pos, scopeLineNumber, file)) {
            bracketStack.pop();
            if(bracketStack.size() == 0) {
              scope.characterNumberEnd = pos;
              scope.lineNumberEnd = scopeLineNumber;
              j = file.lines.size();
              break;
            }
          }
        } else if(bracketStack.size() == 0 && c == ';') {
          //Should only apply for conditonal and variable scope
          scope.characterNumberEnd = pos;
          scope.lineNumberEnd = scopeLineNumber;
          j = file.lines.size();
          break;
        }
        
        if(bracketStack.size() > BRACKET_STACK_GIVEUP){
          throw std::overflow_error("Reached maximum bracket size, something is probably wrong");
        }
      }
      startingCharForThisLine = 0;
    }

    return scopeLineNumber;
  }


  int CppScopeExtractor::findEndOfScopeConditionalFor(Scope& scope, File& file, int startingLine, int startingCharacter) {
    int startingCharForThisLine = startingCharacter;
    std::stack<char> ParenthesisStack;

    for(unsigned int j = startingLine; j < file.lines.size(); ++j) {
      const std::string& namespaceLine = file.lines[j];
      for(unsigned int pos = startingCharForThisLine; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        if(c == '(') {
          if(!isWithinStringLiteral(pos, j, file)) {
            ParenthesisStack.push('(');
          }
        } else if(c == ')') {
          if(!isWithinStringLiteral(pos, j, file)) {
            ParenthesisStack.pop();
            if(ParenthesisStack.size() == 0) {
              findEndOfScope(scope, file, j, pos);
              j = file.lines.size();
              break;
            }
          }
        }
        
        if(ParenthesisStack.size() > BRACKET_STACK_GIVEUP){
          throw std::overflow_error("Reached maximum bracket size, something is probably wrong");
        }
      }
      startingCharForThisLine = 0;
    }
    
    
    return startingCharForThisLine;
  }


  int CppScopeExtractor::findEndOfScopeConditionalDoWhile(Scope& scope, File& file, int startingLine) {
    int counter = 0;
    std::regex conditionalRegex(R"(\s*(while|do)(\(|\{|\s|$))");
    for(unsigned int i = startingLine; i < file.lines.size(); ++i) {
      const std::string& line = file.lines[i];
      std::smatch match;
      std::regex_search(line, match, conditionalRegex);
      if(match.size() > 0) {
        if(match[1] == "do") {
          counter++;
        } else if(match[1] == "while") {
          if(isDoWhileLoop(file, i, line.find(match[0]))) {
            if(counter > 0) {
              counter--;
            } else {
              for(unsigned int pos = 0; pos < line.size(); ++pos) {
                const char& c = line[pos];
                if(c == ';') {
                  scope.characterNumberEnd = pos;
                  scope.lineNumberEnd = i;
                  return 0;
                }
              }
            }
          }
        }
      }
    }
    return counter;
  }

  bool CppScopeExtractor::isDoWhileLoop(File& file, int startingLine, int startingCharacter) {
    int startingCharForThisLine = startingCharacter;
    bool foundCondition = false;
    std::stack<char> ParenthesisStack;
    for(unsigned int j = startingLine; j < file.lines.size(); ++j) {
      const std::string& namespaceLine = file.lines[j];
      for(unsigned int pos = startingCharForThisLine; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        
        if(ParenthesisStack.size() > BRACKET_STACK_GIVEUP){
          throw std::overflow_error("Reached maximum parenthesis size, something is probably wrong");
        }
        
        if(!foundCondition) {
          if(c == '(') {
            if(!isWithinStringLiteral(pos, j, file)) {
              ParenthesisStack.push('(');
            }
          } else if(c == ')') {
            if(!isWithinStringLiteral(pos, j, file)) {
              ParenthesisStack.pop();
              if(ParenthesisStack.size() == 0) {
                foundCondition = true;
              }
            }
          }
        } else {
          if(!isspace(c)) {
            if(c == ';') {
              return true;
            } else {
              return false;
            }
          }
        }
      }
      startingCharForThisLine = 0;
    }
    return false;
  }

  bool CppScopeExtractor::isLineWithinDefine(const std::string& filename, int lineNumber){
    for(const auto& scope : m_defineScopes[filename]){
      if(scope.isLineWithinScope(lineNumber)){
        return true;
      }
    }
    
    return false;
  }
  
}
