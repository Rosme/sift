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

#pragma once
#include <memory>
#include <functional>
#include <map>
#include <utility>
#include <thread>
#include <atomic>
#include <mutex>

#include "core/constants.hpp"
#include "core/message_stack.hpp"
#include "core/file.hpp"
#include "core/scope.hpp"
#include "core/scope_extractor.hpp"
#include "syntax/rule.hpp"
#include "syntax/syntax_analyser.hpp"
#include "flow/flow_analyser.hpp"

// Class to render the function flow explicit, better than to have everything laid out in main
class SIFT
{
public:
  SIFT();
    
  // Move these to syntax_analyzer accordingly
  void parseArgv(int argc, char** argv);
  void setupLogging();
  void setupRules(const std::string filename);
  void setupRules(std::map<RuleId, Syntax::Rule> rules);
  void extractScopes();
  void applyRules();
  void registerConflictDefinitions();
  void registerRuleWork();
  void outputMessagesSyntax(long long executionTime);
  void outputMessagesFlow(long long executionTime);
  void readPath(const std::string& path);
  void clearState();
  void verifyFlow();
    
  const std::map<const std::string, Core::MessageStack> getMessageStacks(){ return m_messageStacks; }
  const std::map<const std::string, Core::MessageStack> getMessageStacksFlow() { return m_messageStacksFlow; }
  std::map<std::string, Core::Scope> getScopes() { return m_rootScopes; }
  std::string getRuleFileName() { return m_ruleFilename; }
  std::string getPathToParse() { return m_pathToParse; }
  
  const std::map<RuleId, Syntax::Rule>& getRules() { return m_rules; }
  void readSource(const std::string & filename, const std::vector<std::string>& source);
private:
  // filename : rawText
  //std::map<std::string, Core::File> m_files;
  std::vector<std::pair<std::string, Core::File>> m_files;
    
  // filename : rootScope
  std::map<std::string, Core::Scope> m_rootScopes;
    
  // id : rule
  std::map<RuleId, Syntax::Rule> m_rules;
    
  // ruleType : work
  std::map<Syntax::RuleType, std::function<void(Syntax::Rule&, Core::Scope&, Core::MessageStack&)>> m_rulesWork;
    
  std::unique_ptr<Flow::FlowAnalyser> m_flowAnalyser;
  std::unique_ptr<Syntax::SyntaxAnalyser> m_syntaxAnalyser;
  std::unique_ptr<Core::ScopeExtractor> m_scopeExtractor;
  std::map<const std::string, Core::MessageStack> m_messageStacks;
  std::map<const std::string, Core::MessageStack> m_messageStacksFlow;
  std::map<Syntax::RuleType, std::vector<Syntax::RuleType>> m_conflict_definitions;
    
  bool m_quietMode;
  bool m_verboseMode;
  std::string m_outputFilename;
  std::string m_loggingSettingsFilename;
  std::string m_ruleFilename;
  std::string m_pathToParse;
  
  void readSingleSourceFile(const std::string& filename);
  void readFilesFromDirectory(const std::string& directory, const std::string& extensions);
  
  std::atomic<int> m_parsingErrors;
  std::atomic<int> m_scopedFileExtracted;
  std::mutex m_mutex;
  using VectorPairFile = std::vector<std::pair<std::string, Core::File*>>;
  void extractScopesImpl(const VectorPairFile& paths);
  VectorPairFile getFileRange(unsigned int start, unsigned int count);
};
