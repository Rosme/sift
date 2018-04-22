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

#include <chrono>
#include <stack>
#include <utility>
#include <muflihun/easylogging++.h>
#include <cxxopts/cxxopts.hpp>
#include <nbsdx/ThreadPool.h>
#include <algorithm>

#include "sift.hpp"
#include "core/config.hpp"
#include "core/assert.hpp"
#include "core/cpp_scope_extractor.hpp"
#include "flow/cpp_flow_analyser.hpp"
#include "syntax/cpp_syntax_analyser.hpp"
#include "syntax/rule.hpp"


SIFT::SIFT()
{
  m_syntaxAnalyser = std::make_unique<Syntax::CPPSyntaxAnalyser>();
  m_flowAnalyser = std::make_unique<Flow::CPPFlowAnalyser>();
  m_scopeExtractor = std::make_unique<Core::CppScopeExtractor>();
  
  registerConflictDefinitions();

  m_parsingErrors = 0;
  m_scopedFileExtracted = 0;
}
  
#define CXXOPT(longName, variableName, type, defaultValue) if(result.count(longName)) { \
  variableName = result[longName].as<type>(); \
}else{ \
  variableName = defaultValue; \
}
  
void SIFT::parseArgv(int argc, char** argv)
{
  cxxopts::Options options("PFE", "Syntax analyser for cpp");
    
  // How does this even work haha
  options.add_options()
  ("v,version", "Display version information")
  ("V,verbose", "Enable verbose mode")
  ("q,quiet", "Enable quiet mode")
  ("l,logconfig", "Specify a easylogging config file to use", cxxopts::value<std::string>())
  ("r,rules", "Specify a rule file to use", cxxopts::value<std::string>())
  ("o,output", "Output results to file", cxxopts::value<std::string>())
  ("h,help", "Print help")
  ("p,path", "Specify what path/filename to parse", cxxopts::value<std::string>(m_pathToParse))
  ;
  try
  {
    options.parse_positional({"path"}); // ./pfe <path>
    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }
    
    if (result.count("version"))
    {
      std::cout << PROGRAM_NAME << ", version: " << VERSION << std::endl;
      exit(0);
    }
    CXXOPT("verbose", m_verboseMode, bool, false);
    CXXOPT("quiet", m_quietMode, bool, false);
    CXXOPT("output", m_outputFilename, std::string, "output.txt");
    CXXOPT("logconfig", m_loggingSettingsFilename, std::string, "samples/logging.conf");
    CXXOPT("rules", m_ruleFilename, std::string, "samples/rules/rules.json");
    CXXOPT("path", m_pathToParse, std::string, "samples/src/brightness_manager.cc");
  }
  catch(...)
  {
    std::cout << options.help({"", "Group"}) << std::endl;
    exit(EXIT_FAILURE);
  }
}
  
void SIFT::setupLogging()
{
  // Setup defaults
  el::Configurations conf;
  conf.setToDefault();
  conf.setGlobally(el::ConfigurationType::Format, "%level | %msg");
  
  if(m_quietMode) {
    conf.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
    conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
  }
  else {
    conf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
    conf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
  }
  
  if(m_verboseMode) {
    conf.set(el::Level::Trace, el::ConfigurationType::Enabled, "true");
  }else{
    conf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
  }
    
  std::ifstream file(m_loggingSettingsFilename);
    
  if(file.good())
  {
    conf = el::Configurations(m_loggingSettingsFilename);
  }
    
  el::Loggers::reconfigureLogger("default", conf);
  
}
  
void SIFT::setupRules(const std::string filename)
{
  m_rules = Syntax::readRules(m_conflict_definitions, filename);
    
  std::stringstream rulesString;
    
  for(auto&& rule : m_rules)
  {
    rulesString << "\n " << rule.second;
  }
    
  LOG(INFO) << "Parsed " << m_rules.size() << " rules:";
  LOG(INFO) << rulesString.str();
}


void SIFT::setupRules(std::map<RuleId, Syntax::Rule> rules){  
  std::stringstream rulesString;
  m_rules = rules;
  
  for(auto&& rule : m_rules)
  {
    rulesString << "\n " << rule.second;
  }
  
  LOG(INFO) << "Gave " << m_rules.size() << " rules:";
  LOG(INFO) << rulesString.str();
}

void SIFT::readSource(const std::string& filename, const std::vector<std::string>& source){
  Core::File file;
  file.filename = filename;
  file.lines = source;
  //m_files[filename] = file;
  m_files.push_back(std::make_pair(filename, file));
  LOG(INFO) << "Source file '" << filename << "' has been read";
}

void SIFT::readSingleSourceFile(const std::string & filename)
{
  Core::File file;
  if(!Core::readSourceFile(filename, file)) {
    LOG(ERROR) << "Could not read source file '" << filename << "'";
    return;
  }

  //m_files[filename] = file;
  m_files.push_back(std::make_pair(filename, file));
  LOG(INFO) << "Source file '" << filename << "' has been read";
}
  
void SIFT::readFilesFromDirectory(const std::string& directory, const std::string& extensions)
{   
  std::vector<std::string> valid_extensions = Core::split(extensions, '|');
  
  SIFT_ASSERT(valid_extensions.size() > 0, "Not enough valid file extensions provided");
  
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
        bool found = false;
        for(auto&& ext : valid_extensions){
          if(Core::string_ends_with(item.fullPath, "." + ext)){
            found = true;
            break;
          }
        }
        if(!found){
          continue;
        }
        
        Core::File file;
        bool success = Core::readSourceFile(item.fullPath, file);
        if(!success)
        {
          LOG(ERROR) << "Could not read source file '" << item.fullPath << "'";
          continue;
        }
          
        //m_files[item.fullPath] = file;
        m_files.push_back(std::make_pair(item.fullPath, file));
      }
    }
  }
    
  LOG(INFO) << "Read " << m_files.size() << " source files";
}
  
void SIFT::extractScopes()
{
  //TODO: Interesting, but could be maybe be a fix at 8?
  std::random_shuffle(m_files.begin(), m_files.end());
  const auto MaxThreadCount = std::thread::hardware_concurrency();
  unsigned int filesPerThread = (m_files.size() / MaxThreadCount)+1;
  
  LOG(INFO) << "Parsing " << filesPerThread << " files per thread";
  
  using nbsdx::concurrent::ThreadPool;

  ThreadPool<8> pool;

  for(int i = 0; i < MaxThreadCount; ++i) {
    pool.AddJob([this, filesPerThread, i]() {
      extractScopesImpl(getFileRange(i*filesPerThread, filesPerThread));
    });
  }

  pool.JoinAll(true);

  for(auto& scopePair : m_rootScopes) {
    m_scopeExtractor->constructTree(scopePair.second);
  }

  if(m_files.size() > 0){
    LOG(INFO) << ">" << m_parsingErrors << "/" << m_files.size() << "< unparsed files (" << std::setprecision(4) << ((float)m_parsingErrors/m_files.size())*100 << "%)";
  }

  LOG(TRACE) << "Extracted " << m_rootScopes.size() << " root scopes";
}

void SIFT::applyRules()
{
  {
    //Clearing Duplicate Rules
    std::vector<Syntax::Rule> ruleList;
    for(auto it = m_rules.begin(); it != m_rules.end();) {
      if(std::find_if(ruleList.begin(), ruleList.end(), [it](const Syntax::Rule& rule) {
        return it->second == rule;
      }) != ruleList.end()) {
        it = m_rules.erase(it);
      } else {
        ruleList.push_back(it->second);
        ++it;
      }
    }
  }

  for(auto& scopePair : m_rootScopes)
  {
    for(auto& rulePair : m_rules)
    {
      auto ruleType = rulePair.second.getRuleType();
      auto it = m_rulesWork.find(ruleType);
      if(it != m_rulesWork.end())
      {
        m_rulesWork[ruleType](rulePair.second, scopePair.second, m_messageStacks[scopePair.second.file->filename]);
      }
    }
  }
}

// TODO maybe have a file for this, but since we aimed for a 'single binary' approach, here's data in code
void SIFT::registerConflictDefinitions()
{
  using namespace Syntax;
  m_conflict_definitions[RuleType::CurlyBracketsCloseSameLine] = {RuleType::CurlyBracketsCloseSeparateLine};
  m_conflict_definitions[RuleType::CurlyBracketsCloseSeparateLine] = {RuleType::CurlyBracketsCloseSameLine};
  
  m_conflict_definitions[RuleType::CurlyBracketsOpenSameLine] = {RuleType::CurlyBracketsOpenSeparateLine};
  m_conflict_definitions[RuleType::CurlyBracketsOpenSeparateLine] = {RuleType::CurlyBracketsOpenSameLine};
  
  m_conflict_definitions[RuleType::OwnHeaderBeforeStandard] = {RuleType::StandardHeaderBeforeOwn};
  m_conflict_definitions[RuleType::StandardHeaderBeforeOwn] = {RuleType::OwnHeaderBeforeStandard};
  
  m_conflict_definitions[RuleType::StartWithLowerCase] = {RuleType::StartWithUpperCase, RuleType::StartWithX};
  m_conflict_definitions[RuleType::StartWithUpperCase] = {RuleType::StartWithLowerCase, RuleType::StartWithX};
  m_conflict_definitions[RuleType::StartWithX] = {RuleType::StartWithLowerCase, RuleType::StartWithUpperCase};
  
}

void SIFT::registerRuleWork()
{
  m_syntaxAnalyser->registerRuleWork(m_rulesWork, m_scopeExtractor->getStringLiterals(), m_scopeExtractor->getComments());
}
  
  
#define OUTPUT(msg) file << msg << "\n"; \
  if(!m_quietMode) \
  { \
    LOG(INFO) << msg; \
  }
  
void SIFT::outputMessagesSyntax(long long executionTime)
{    
  auto findReplaceFn = [&](std::string& from, const std::string find, const std::string replace){
    auto index = from.find(find);
    if(index != std::string::npos){
      from.replace(index, find.length(), replace);
    }
  };
  
  std::ofstream file(m_outputFilename);
  // Files
  for(auto&& stackPair : m_messageStacks) {
    if(stackPair.second.size() == 0)
    {
      continue; // Don't bother displaying error-free files
    }
    
    OUTPUT("+ " << stackPair.first << " ----------");

    // Rules
    for(const auto& ruleIdMessagesPair : stackPair.second.getMessages()) {
      const Syntax::Rule& rule = m_rules[ruleIdMessagesPair.first];
      
      std::string ruleString = m_syntaxAnalyser->getRuleMessage(rule);
      
      std::string ruleName = Syntax::to_string(rule.getRuleType());
      std::string ruleScope = Core::to_string(rule.getScopeType());
      std::string ruleParameter = rule.getParameter();
            
      findReplaceFn(ruleString, "%rs", ruleScope);      
      findReplaceFn(ruleString, "%rp", ruleParameter.empty() ? "" : ruleParameter);

      OUTPUT("  " << ruleName << " -- " << ruleString);
      
      for(const auto& message : ruleIdMessagesPair.second) {
        OUTPUT("    " << message);
      }
    }
  }
  
  OUTPUT("Syntax Ran in " << executionTime << "ms");

  file.close();
    
  LOG(INFO) << "Wrote results to file: " << m_outputFilename;
}

void SIFT::outputMessagesFlow(long long executionTime)
{
  auto findReplaceFn = [&](std::string& from, const std::string find, const std::string replace) {
    auto index = from.find(find);
    if (index != std::string::npos) {
      from.replace(index, find.length(), replace);
    }
  };

  std::ofstream file(m_outputFilename, std::ios::app);
  // Files
  for (auto&& stackPair : m_messageStacksFlow) {
    if (stackPair.second.size() == 0)
    {
      continue; // Don't bother displaying error-free files
    }

    OUTPUT("+ " << stackPair.first << " ----------");

    // Rules
    for (const auto& ruleIdMessagesPair : stackPair.second.getMessages()) {

      for (const auto& message : ruleIdMessagesPair.second) {
        OUTPUT("    " << message);
      }
    }
  }

  OUTPUT("Flow Ran in " << executionTime << "ms");

  file.close();

  LOG(INFO) << "Wrote results to file: " << m_outputFilename;
}

void SIFT::readPath(const std::string& path)
{
  LOG(INFO) << "Reading path " << path;
  if(Core::directoryExists(path)){
    readFilesFromDirectory(path, "cpp|hpp|h|c|cc|hh"); //TODO standardized way
  }else{
    readSingleSourceFile(path);
  }
}

void SIFT::clearState(){
  m_rootScopes.clear();
  m_files.clear();
  m_rules.clear();
  m_rulesWork.clear();
}

void SIFT::verifyFlow() {
  for (auto& scopePair : m_rootScopes) {
    m_flowAnalyser->analyzeFlow(scopePair.second, m_messageStacksFlow[scopePair.second.file->filename]);
  }
}

void SIFT::extractScopesImpl(const SIFT::VectorPairFile& files) {
  
  for(const auto& filePair : files) {
    Core::Scope scope;
    ++m_scopedFileExtracted;
    LOG(INFO) << "[" << m_scopedFileExtracted << "/" << m_files.size() << "] Parsing " << filePair.second->filename;
    bool success = m_scopeExtractor->extractScopesFromFile(*filePair.second, scope);
    if(success)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_rootScopes[filePair.first] = scope;
    }
    else
    {
      LOG(ERROR) << "Could not parse source file '" << filePair.first << "'";
      ++m_parsingErrors;
    }
  }
}

SIFT::VectorPairFile SIFT::getFileRange(unsigned int start, unsigned int count) {
  VectorPairFile files;
  
  for(unsigned int i = start; i < start+count && i < m_files.size(); ++i) {
    auto& pa = m_files[i];
    files.push_back(std::make_pair(pa.first, &pa.second));
  }
  
  return files;
}
