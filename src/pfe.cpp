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

// Fix Cxxopts under MSVC
#define NOMINMAX

#include <regex>
#include <chrono>
#include <stack>
#include <muflihun/easylogging++.h>
#include <cxxopts/cxxopts.hpp>

#include "core/config.hpp"
#include "pfe.hpp"
#include "flow/cpp_flow_analyser.hpp"
#include "syntax/cpp_syntax_analyser.hpp"
#include "syntax/rule.hpp"

namespace Core
{
  PFE::PFE()
  {
    m_syntaxAnalyser = std::make_unique<Syntax::CPPSyntaxAnalyser>();
    m_flowAnalyser = std::make_unique<Flow::CPPFlowAnalyser>();
  }
  
  #define CXXOPT(longName, variableName, type, defaultValue) if(result.count(longName)) { \
    variableName = result[longName].as<type>(); \
  }else{ \
    variableName = defaultValue; \
  }
  
  void PFE::parseArgv(int argc, char** argv)
  {
    cxxopts::Options options("PFE", "Syntax analyser for cpp");
    
    // How does this even work haha
    options.add_options()
    ("d,debug", "Enable debugging")
    ("q,quiet", "Enable quiet mode")
    ("l,logconfig", "Specify a easylogging config file to use", cxxopts::value<std::string>())
    ("o,output", "Output results to file", cxxopts::value<std::string>())
    ("h,help", "Print help")
    ;
    try
    {
      auto result = options.parse(argc, argv);
      if (result.count("help"))
      {
        std::cout << options.help({"", "Group"}) << std::endl;
        exit(0);
      }
      
      CXXOPT("quiet", m_quietMode, bool, false);
      CXXOPT("output", m_outputFilename, std::string, "output.txt");
      CXXOPT("logconfig", m_loggingSettingsFilename, std::string, "samples/logging.conf");
    }
    catch(...)
    {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  
  void PFE::setupLogging()
  {
    // Setup defaults
    el::Configurations conf;
    conf.setToDefault();
    conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
    conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    conf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
    
    std::ifstream file(m_loggingSettingsFilename);
    
    if(file.good())
    {
      conf = el::Configurations(m_loggingSettingsFilename);
    }
    
    el::Loggers::reconfigureLogger("default", conf);
  }
  
  void PFE::setupRules(const std::string filename)
  {
    m_rules = Syntax::readRules(filename);
    
    std::stringstream rulesString;
    
    for(auto&& rule : m_rules)
    {
      rulesString << rule.second << ", ";
    }
    
    LOG(INFO) << "Parsed " << m_rules.size() << " rules:";
    LOG(INFO) << rulesString.str();
  }

  void PFE::readSingleSourceFile(const std::string & filename) 
  {
    Core::File file;
    if(!Core::readSourceFile(filename, file)) {
      LOG(ERROR) << "Could not read source file '" << filename << "'";
      return;
    }

    m_files[filename] = file;
    LOG(INFO) << "Source file '" << filename << "' has been read";
  }
  
  void PFE::readFilesFromDirectory(const std::string& directory)
  {   
    std::vector<Core::FilesystemItem> stack, current, all;
    stack = Core::getFilenamesInDirectory(directory);
    
    while(stack.size() > 0)
    {
      current = stack;
      stack.clear();
      for(auto&& item : current)
      {
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
          
          m_files[item.fullPath] = file;
        }
      }
    }
    
    LOG(INFO) << "Read " << m_files.size() << " source files";
  }
  
  void PFE::extractScopes()
  {
    // Parse all files found
    for(auto&& filePair : m_files)
    {
      Core::Scope scope;
      bool success = extractScopesFromFile(filePair.second, scope);
      if(success)
      {
        m_rootScopes[filePair.first] = scope;
      }
      else
      {
        LOG(ERROR) << "Could not parse source file '" << filePair.first << "'";
      }
    }
    
    LOG(INFO) << "Extracted " << m_rootScopes.size() << " root scopes";
  }
  
  bool PFE::extractScopesFromFile(Core::File& file, Core::Scope &outScope)
  {
    Core::Scope rootScope(Core::ScopeType::Source);
    rootScope.file = &file;
    rootScope.name = file.filename;
    rootScope.lineNumberStart = 0;
    rootScope.lineNumberEnd = file.lines.size();
    rootScope.characterNumberStart = 0;
    rootScope.characterNumberEnd = 0;

    //The idea here is to fill the rootscope and have all the scopes on a flat line at no depth
    //Afterward, we will construct the tree correctly based on analysis of which scope is within whom
    //This allows us to not have much recursion and handle pretty much all edge case of scope within scopes.
    extractGlobals(file, rootScope);
    extractNamespaces(file, rootScope);
    extractEnums(file, rootScope);
    extractClasses(file, rootScope);
    extractFunctions(file, rootScope);
    extractVariables(file, rootScope);
	  extractConditionals(file, rootScope);
    constructTree(rootScope);

    LOG(INFO) << "\n" << rootScope.getTree();

    outScope = rootScope;
    
    return true;
  }
  
  void PFE::extractGlobals(Core::File& file, Core::Scope& parent)
  {
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
        std::regex defineRegex("^(\\s*|\\s*\\/\\*.*\\*\\/\\s*)#define");
        std::smatch sm;
        std::regex_search(line, sm, defineRegex);
        if(sm.size() > 0)
        {
          scope = Core::Scope(Core::ScopeType::GlobalDefine);
          scope.lineNumberStart = lineNo;
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
            scope.name = line;
            parent.children.push_back(scope);
          }
        }
      }
      else if(isStillInDefine)
      {
        if(line.find("\\") == std::string::npos)
        {
          scope.characterNumberEnd = charNo + line.size()-1;
          scope.name = scope.getScopeLines().at(0);
          parent.children.push_back(scope);
          isStillInDefine = false;
        }
      }
      lineNo++;
      charNo+=line.size()+1;
    }
    
  }

  void PFE::extractNamespaces(Core::File& file, Core::Scope& parent) {
    // Regex that match namespace without using in front
    // The regex supports space or no space after the name, and any kind of return line (UNIX/Windows)
    std::regex namespaceRegex("^(?!using)\\s*namespace (\\w*)(\\n|\\r\\n)*\\s*(\\{*)");
    for(int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, namespaceRegex);
      if(match.size() > 0) {
        Core::Scope scope(Core::ScopeType::Namespace);
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

  void PFE::extractEnums(Core::File& file, Core::Scope& parent) {
    std::regex enumRegex("\\s*enum(\\s*class)?\\s*(\\w*)\\s*:?\\s*(\\w|\\s)*\\s*\\{?");
    for(int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, enumRegex);

      if(match.size() > 0) {
        Core::Scope scope(Core::ScopeType::Enum);
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

  void PFE::extractClasses(Core::File & file, Core::Scope & parent) {
    std::regex classRegex("(?!enum)\\s*(class|struct)\\s*(\\w*)\\s*:?\\s*(\\w|\\s)*\\s*\\{?");
    for(int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, classRegex);
      if(match.size() > 0) {
        Core::Scope scope(Core::ScopeType::Class);
        scope.name = match[2];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.file = &file;

        //Struct/Class can have Struct/Class inside
        findEndOfScope(scope, file, lineNumber);

        LOG(DEBUG) << "\n" << scope;

        parent.children.push_back(scope);
      }
    }
  }

  void PFE::extractFunctions(Core::File & file, Core::Scope & parent) {
    std::regex functionRegex("(\\w*\\s)*((\\w|:)+)\\s*\\((.*),?\\).*");
    for(int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, functionRegex);
      if(match.size() > 0) {
        Core::Scope scope(Core::ScopeType::Function);
        scope.name = match[2];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.file = &file;

        std::size_t declarationEnd = line.find(';',scope.characterNumberStart);
        if(declarationEnd != std::string::npos) {
          scope.characterNumberEnd = declarationEnd;
          scope.lineNumberEnd = scope.lineNumberStart;
        } else {
          findEndOfScope(scope, file, lineNumber);
        }

        LOG(DEBUG) << "\n" << scope;

        parent.children.push_back(scope);
      }
    }
  }

  void PFE::extractVariables(Core::File & file, Core::Scope & parent) {
    std::regex variableRegex("\\s*(?!return)((\\w+\\*?\\s+)|(\\w+\\s+\\*?)|(\\w+\\s+\\*\\s+))(\\w+)\\s*(=\\s*\\w*)?\\s*;");
    for(int lineNumber = parent.lineNumberStart; lineNumber < parent.lineNumberEnd; ++lineNumber) {
      const std::string& line = file.lines[lineNumber];
      std::smatch match;
      std::regex_match(line, match, variableRegex);
      if(match.size() > 0) {
        Core::Scope scope(Core::ScopeType::Variable);
        scope.name = match[5];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.lineNumberEnd = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.characterNumberEnd = line.find(';', scope.characterNumberStart);
        scope.file = &file;

        LOG(INFO) << "\n" << scope;

        //Adding to parent the scope
        parent.children.push_back(scope);
      }
    }
  }

  void PFE::extractConditionals(Core::File& file, Core::Scope& parent) {
    std::regex conditionnalRegex(R"(\s*(if|else|for|switch|while|do)(\(|\{|\s|$))");
    for (int i = parent.lineNumberStart; i < parent.lineNumberEnd; ++i) {
      int lineNumber = i + 1;
      const std::string& line = file.lines[i];
      std::smatch match;
      std::regex_search(line, match, conditionnalRegex);
      if (match.size() > 0) {
        if (match[1] == "while") {
          if (isDoWhileLoop(file, i, line.find(match[0]))) {
            continue;
          }
        }
        Core::Scope scope(Core::ScopeType::Conditionnal);
        scope.name = match[1];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.file = &file;

        if (scope.name == "for") {
          findEndOfScopeConditionalFor(scope, file, i, scope.characterNumberStart);
        } else if (match[1] == "do") {
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

  bool PFE::isDoWhileLoop(Core::File& file, int startingLine, int startingCharacter) {
    int startingCharForThisLine = startingCharacter;
    int scopeLineNumber = startingLine;
    bool foundCondition = false;
    std::stack<char> ParenthesisStack;
    for (int j = startingLine; j < file.lines.size(); ++j) {
      scopeLineNumber = j + 1;
      const std::string& namespaceLine = file.lines[j];
      for (unsigned int pos = startingCharForThisLine; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        if (!foundCondition) {
          if (c == '(') {
            ParenthesisStack.push('(');
          }
          else if (c == ')') {
            ParenthesisStack.pop();
            if (ParenthesisStack.size() == 0) {
              foundCondition = true;
            }
          }
        }
        else {
          if (c != ' ' && c != '\r' && c != '\n') {
            if (c == ';') {
              return true;
            }
            else {
              return false;
            }
          }
        }
      }
      startingCharForThisLine = 0;
    }
    return false;
  }


  void PFE::constructTree(Core::Scope& root) {
    //Finding the parent of every Scope
    for(auto it = root.children.rbegin(); it != root.children.rend(); it++) {
      if(it->type == ScopeType::GlobalDefine){
        it->parent = &root;
        continue;
      }
      
      Core::Scope& bestParent = findBestParent(root, *it);
      it->parent = &bestParent;
      //Checking if it's a variable or function, and being more precise about it if it's the case
      if(it->type == ScopeType::Variable) {
        if(bestParent.type == ScopeType::Source || bestParent.type == ScopeType::Namespace) {
          it->type = ScopeType::GlobalVariable;
        } else if(bestParent.type == ScopeType::Conditionnal || bestParent.type == ScopeType::Function) {
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
          } else if(static_cast<unsigned int>(bestParent.type & ScopeType::Function) != 0) {
            //Declared like a function(e.g. int var(5))
            it->type = ScopeType::FunctionVariable;
          } else {
            it->type = ScopeType::FreeFunction;
          }
        }
      }
    }

    //Recreating the tree with the children
    /*for(int i = 0; i < root.children.size(); ++i) {
      auto& scope = root.children[i];

      if(scope.parent && scope.parent->type != ScopeType::Source) {
        auto& parentChildren = scope.parent->children;
        bool isAlreadyIn = false;
        for(int j = 0; j < parentChildren.size(); ++j) {
          if(parentChildren[j] == scope) {
            isAlreadyIn = true;
            break;
          }
        }
        if(!isAlreadyIn) {
          parentChildren.push_back(scope);
          i = -1;
        }
      }
    }*/

    //Removing superfluous scopes
    /*for(auto it = root.children.begin(); it != root.children.end(); ++it) {
      if(it->parent->type != ScopeType::Source) {
        root.children.erase(it);
        it = root.children.begin();
      }
    }*/
  }

  Core::Scope& PFE::findBestParent(Core::Scope& root, Core::Scope& toSearch) {
    Core::Scope* bestCandidate = &root;

    for(int i = 0; i < root.children.size(); ++i) {
      Scope& child = root.children[i];
      if(toSearch.isWithinOtherScope(child) && toSearch != child) {
        if(child.isWithinOtherScope(*bestCandidate)) {
          bestCandidate = &child;
        }
      }
    }

    return *bestCandidate;
  }

  int PFE::findEndOfScope(Core::Scope& scope, Core::File& file, int startingLine) {
    //TODO: Review this. Not really happy with this.
    std::stack<char> bracketStack;
    int offset = scope.characterNumberStart;
    int scopeLineNumber = startingLine;
    for(int j = startingLine; j < file.lines.size(); ++j) {
      scopeLineNumber = j+1;
      const std::string& namespaceLine = file.lines[j];
      for(unsigned int pos = 0; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        if(c == '{') {
          bracketStack.push('{');
        } else if(c == '}') {
          bracketStack.pop();
          if(bracketStack.size() == 0) {
            scope.characterNumberEnd = pos;
            scope.lineNumberEnd = scopeLineNumber;
            j = file.lines.size();
            break;
          }
        }
      }
    }

    return scopeLineNumber;
  }

  int PFE::findEndOfScope(Core::Scope& scope, Core::File& file, int startingLine, int startingCharacter) {
    std::stack<char> bracketStack;
    int offset = scope.characterNumberStart;
    int scopeLineNumber = startingLine;
    int startingCharForThisLine = startingCharacter;
    for (int j = startingLine; j < file.lines.size(); ++j) {
      scopeLineNumber = j + 1;
      const std::string& namespaceLine = file.lines[j];
      for (unsigned int pos = startingCharForThisLine; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        if (c == '{') {
          bracketStack.push('{');
        } else if (c == '}') {
          bracketStack.pop();
          if (bracketStack.size() == 0) {
            scope.characterNumberEnd = pos;
            scope.lineNumberEnd = scopeLineNumber;
            j = file.lines.size();
            break;
          }
        } else if (bracketStack.size() == 0 && c == ';') {
          //Should only apply for conditonal and variable scope
          scope.characterNumberEnd = pos;
          scope.lineNumberEnd = scopeLineNumber;
          j = file.lines.size();
          break;
        }
      }
      startingCharForThisLine = 0;
    }

    return scopeLineNumber;
  }


  int PFE::findEndOfScopeConditionalFor(Core::Scope& scope, Core::File& file, int startingLine, int startingCharacter) {
    int startingCharForThisLine = startingCharacter;
    int scopeLineNumber = startingLine;
    std::stack<char> ParenthesisStack;
    for (int j = startingLine; j < file.lines.size(); ++j) {
      scopeLineNumber = j + 1;
      const std::string& namespaceLine = file.lines[j];
      for (unsigned int pos = startingCharForThisLine; pos < namespaceLine.size(); ++pos) {
        const char& c = namespaceLine[pos];
        if (c == '(') {
          ParenthesisStack.push('(');
        }
        else if (c == ')') {
          ParenthesisStack.pop();
          if (ParenthesisStack.size() == 0) {
            findEndOfScope(scope, file, j, pos);
            j = file.lines.size();
            break;
          }
        }
      }
      startingCharForThisLine = 0;
    }
    return startingCharForThisLine;

  }


  int PFE::findEndOfScopeConditionalDoWhile(Core::Scope& scope, Core::File& file, int startingLine) {
    int counter = 0;
    std::regex conditionnalRegex(R"(\s*(while|do)(\(|\{|\s|$))");
    for (int i = startingLine; i < file.lines.size(); ++i) {
      const std::string& line = file.lines[i];
      std::smatch match;
      std::regex_search(line, match, conditionnalRegex);
      if (match.size() > 0) {
        if (match[1] == "do")
        {
          counter++;
        }
        else if (match[1] == "while")
        {
          if (isDoWhileLoop(file, i, line.find(match[0])))
          {
            if (counter > 0)
            {
              counter--;
            }
            else
            {
              for (unsigned int pos = 0; pos < line.size(); ++pos) {
                const char& c = line[pos];
                if (c == ';') {
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
  }

  void PFE::applyRules()
  {
    for(auto& scopePair : m_rootScopes)
    {
      for(auto& rulePair : m_rules)
      {
        auto it = m_rulesWork.find(rulePair.first);
        if(it != m_rulesWork.end())
        {
          m_rulesWork[rulePair.first](rulePair.second, scopePair.second, m_messageStack);
        }
      }
    }
  }
  

  void PFE::registerRuleWork()
  {
    m_syntaxAnalyser->registerRuleWork(m_rulesWork);
  }
  
  void PFE::outputMessages()
  {    
    std::ofstream file(m_outputFilename);
    while(m_messageStack.hasMessages())
    {
      auto message = m_messageStack.popMessage();
      file << message << "\n";
      if(!m_quietMode)
      {
        LOG(INFO) << message;
      }
    }
    
    file.close();
    
    LOG(INFO) << "Wrote results to file: " << m_outputFilename;
  }
};


