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
#include "rule.hpp"
#include "../core/utils.hpp"

namespace Syntax {
  
  std::map<RuleId, Rule> readRules(const std::map<Syntax::RuleType, std::vector<Syntax::RuleType>> &conflicts, const std::string& rulesFile) {
    static long long int currentId = 0;
    std::map<RuleId, Rule> rules;
    
    auto json = Core::readJsonFile(rulesFile);
    
    if(json["rules"].is_array())
    {
      auto jsonRules = json["rules"].get<std::vector<nlohmann::json>>();
      
      for(const auto& jsonRule : jsonRules)
      {
        const std::string parameter = (jsonRule.find("parameter") != jsonRule.end()) ? jsonRule["parameter"].get<std::string>() : "";
        const std::string appliedTo = (jsonRule.find("appliedTo") != jsonRule.end()) ? jsonRule["appliedTo"].get<std::string>() : "All";
        std::string ruleText = jsonRule["rule"].get<std::string>();
        RuleType ruleType = RuleType_to_enum_class(ruleText);
        
        bool found = false;
        for(const auto& ruleNamePair : RuleType_enum_names){
          if(Core::string_case_compare(ruleText, ruleNamePair.second)){
            found = true;
            break;
          }
        }
        if(!found){
          ruleType = RuleType::Unknown;
          LOG(ERROR) << "Rule with name '" << ruleText << "' has not been recognized, it will default to 'Unknown'";
        }
        
        Rule rule(++currentId,
                  Core::ScopeType_to_enum_class(appliedTo),
                  ruleType,
                  parameter);
      
        const auto conflictVectorIt = conflicts.find(rule.getRuleType());
        Syntax::Rule const* ruleInConflict = nullptr;
        if(conflictVectorIt != conflicts.cend())
        {
          const auto& conflictVector = conflictVectorIt->second;
          for(const auto& rulePair : rules)
          {
            const auto& conflictIt = std::find(conflictVector.begin(), conflictVector.end(), rulePair.second.getRuleType());
            if(conflictIt != conflictVector.end())
            {
              // One rule is considering to be clashing if it shares a scope with another one
              bool hasClashingScopes = false;
              
              for(int i = 0; i < 32; i++)
              {
                int currentBit = 1 << i;
                int aBit = (int)rule.getScopeType() & currentBit;
                int bBit = (int)rulePair.second.getScopeType() & currentBit;
                hasClashingScopes = aBit == bBit && aBit != 0;
                
                if(hasClashingScopes)
                {
                  break;
                }
              }
              
              if(hasClashingScopes)
              {
                ruleInConflict = &rulePair.second;
                break;
              }
            } 
          }
          
        }
        
        if(ruleInConflict == nullptr)
        {
          rules[currentId] = rule;
        }
        else
        {
          LOG(ERROR) << rule << " is in conflict with other rule: " << *ruleInConflict << ", it will be dropped";
        }
  
      }
    }
    
    return rules;
  }

  bool operator==(const Rule& lhs, const Rule& rhs) {
    return lhs.getRuleType() == rhs.getRuleType()
      && lhs.getParameter() == rhs.getParameter()
      && lhs.getScopeType() == rhs.getScopeType();
  }
  
  std::ostream& operator<<(std::ostream& out, const Syntax::Rule& rule) {
    out << "Rule: " << to_string(rule.getRuleType()) << " - Applied To: " << Core::to_string(rule.getScopeType());
    if(rule.hasParameter()) {
      out << " - Parameter: " << rule.getParameter();
    }
    return out;
  }
  
  Rule::Rule(RuleId id, Core::ScopeType applyTo, RuleType type, const std::string& optionalParameter)
    : m_id(id)
    , m_applyTo(applyTo)
    , m_type(type)
    , m_parameter(optionalParameter)
  {
  }
  
  Core::ScopeType Rule::getScopeType() const {
    return m_applyTo;
  }
  
  Syntax::RuleType Rule::getRuleType() const {
    return m_type;
  }
  
  RuleId Rule::getRuleId() const
  {
    return m_id;
  }

  bool Rule::hasParameter() const {
    return m_parameter.size() > 0;
  }

  const std::string& Rule::getParameter() const {
    return m_parameter;
  }

}
