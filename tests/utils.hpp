#pragma once

#include <vector>
#include <string>
#include "core/message_stack.hpp"
#include "pfe.hpp"

#define DUMP_STACK(stack) for(auto&& mp : stack.getMessages()){ \
  std::cout << "RuleId: " << mp.first << ", " << pfe.getRules().at(mp.first) << std::endl; \
  for(auto&& m : mp.second) \
  { \
    std::cout << "\t" << m << std::endl;\
  } \
}

inline std::vector<char*> convert(std::vector<std::string>& base)
{
  std::vector<char*> argv;
  for (const auto& arg : base)
    argv.push_back((char*)arg.data());
  argv.push_back(nullptr);
  return argv;
}

// Execute a unit test with the source filename and the .json rules file
inline const Core::MessageStack doTestWithFile(PFE& pfe, const std::string rulesFile, const std::string sourceFile)
{
  pfe.clearState();
  pfe.setupRules(rulesFile);
  pfe.registerRuleWork();
  pfe.readPath(sourceFile);
  pfe.extractScopes();
  pfe.applyRules();
  return pfe.getMessageStacks().at(sourceFile);
}

static RuleId ruleId = 0;
// Execute a unit test with the source filename and the rules as a map
inline const Core::MessageStack doTestWithFile(PFE& pfe, std::map<RuleId, Syntax::Rule> rules, const std::string& sourceFile)
{
  pfe.clearState();
  pfe.setupRules(rules);
  pfe.registerRuleWork();
  pfe.readPath(sourceFile);
  pfe.extractScopes();
  pfe.applyRules();
  return pfe.getMessageStacks().at(sourceFile);
}

// Execute a unit test with the source passed in as a vector of strings and the rules as a map
inline const Core::MessageStack doTestWithSource(PFE& pfe, std::map<RuleId, Syntax::Rule> rules, const std::vector<std::string>& source)
{
  pfe.clearState();
  pfe.setupRules(rules);
  pfe.registerRuleWork();
  pfe.readSource("dummy_filename", source);
  pfe.extractScopes();
  pfe.applyRules();
  return pfe.getMessageStacks().at("dummy_filename");
}

// Test a single line of code
inline const Core::MessageStack doTestWithSource(PFE& pfe, std::map<RuleId, Syntax::Rule> rules, const std::string& sourceLine){
  std::vector<std::string> source = {sourceLine};
  return doTestWithSource(pfe, rules, source);
}
