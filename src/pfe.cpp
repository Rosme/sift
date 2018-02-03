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
    rootScope.lineNumberStart = 0;
    rootScope.lineNumberEnd = file.lines.size();
    rootScope.characterNumberStart = 0;
    rootScope.characterNumberEnd = 0;
   
    // TODO: Review this call. Shouldn't this be part of the rule check? Here for eventual flow replacement?
    //extractDefines(file, rootScope);
    extractNamespaces(file, rootScope);

    outScope = rootScope;
    
    return true;
  }
  
  void PFE::extractDefines(Core::File& file, Core::Scope& outScope)
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
          scope = Core::Scope(Core::ScopeType::Namespace);
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
            outScope.children.push_back(scope);
          }
        }
      }
      else if(isStillInDefine)
      {
        if(line.find("\\") == std::string::npos)
        {
          scope.characterNumberEnd = charNo + line.size()-1;
          outScope.children.push_back(scope);
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
    for(int i = parent.lineNumberStart; i < parent.lineNumberEnd; ++i) {
      int lineNumber = i+1;
      const std::string& line = file.lines[i];
      std::smatch match;
      std::regex_match(line, match, namespaceRegex);
      if(match.size() > 0) {
        Core::Scope scope(Core::ScopeType::Namespace);
        scope.name = match[1];
        scope.parent = &parent;
        scope.lineNumberStart = lineNumber;
        scope.characterNumberStart = line.find(match[0]);
        scope.file = &file;

        //Finding end of namespace
        //TODO: Not really happy with this.
        std::stack<char> bracketStack;
        int offset = scope.characterNumberStart;
        for(int j = i; j < file.lines.size(); ++j) {
          int namespaceLineNumber = j+1;
          const std::string& namespaceLine = file.lines[j];
          for(unsigned int pos = 0; pos < namespaceLine.size(); ++pos) {
            const char& c = namespaceLine[pos];
            if(c == '{') {
              bracketStack.push('{');
            } else if(c == '}') {
              bracketStack.pop();
              if(bracketStack.size() == 0) {
                scope.characterNumberEnd = pos;
                scope.lineNumberEnd = namespaceLineNumber;
                i = namespaceLineNumber;
                j = file.lines.size();
                break;
              }
            }
          }
        }

        LOG(INFO) << "\n" << scope;
        //Time to check if the namespace has a namespace
        extractNamespaces(file, scope);

        //Adding to parent the scope
        parent.children.push_back(scope);
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


