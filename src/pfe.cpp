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
#include <chrono>
#include <muflihun/easylogging++.h>
#include <cxxopts/cxxopts.hpp>

#include "pfe.hpp"
#include "flow/cpp_flow_analyser.hpp"
#include "syntax/cpp_syntax_analyser.hpp"
#include "syntax/rule.hpp"

namespace Core
{
  PFE::PFE()
  {
    //TODO impl
    m_syntaxAnalyser = std::make_shared<Syntax::CPPSyntaxAnalyser>();
    m_flowAnalyser = std::make_shared<Flow::CPPFlowAnalyser>();
  }
  
  void PFE::parseArgv(int argc, char** argv)
  {
    cxxopts::Options options("PFE", "Syntax analyser for cpp");
    
    // How does this even work haha
    options.add_options()
    ("d,debug", "Enable debugging")
    ("q,quiet", "Enable quiet mode")
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
      
      // Uses default if not found
      m_quietMode = result["q"].as<bool>();
      
      if(result.count("output"))
      {
        m_outputFilename = result["output"].as<std::string>();
      }
      else
      {
        m_outputFilename = "output.txt";
      }
    }
    catch(...)
    {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  
  void PFE::setupLogging()
  {
    // This would be 10x better in a log.conf
    el::Configurations conf;
    conf.setToDefault();
    conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
    conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    conf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
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
  
  void PFE::readFilesFromDirectory(const std::string directory)
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
   
    extractDefines(file, rootScope);
    
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
  
  // Force consistency between name and method
  #define NS(ns,item) ns::item
  #define REGISTER_RULE(REG) m_rulesWork[NS(RuleType, REG)] = CPPSyntaxAnalyser::Rule##REG
  void PFE::registerRuleWork()
  {
    using namespace Syntax;
    
    // Registers a rule, expects a name in Syntax::RuleType::RULENAME and a function named CPPSyntaxAnalyser::RuleRULENAME;
    REGISTER_RULE(NoDefine);
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


