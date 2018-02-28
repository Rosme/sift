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

inline const Core::MessageStack doTest(PFE& pfe, const std::string rules, const std::string source)
{
  pfe.setupRules(rules);
  pfe.registerRuleWork();
  pfe.readPath(source);
  pfe.extractScopes();
  pfe.applyRules();
  return pfe.getMessageStacks().at(source);
}
