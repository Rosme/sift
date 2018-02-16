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
  
  std::map<RuleType, Rule> readRules(const std::string& rulesFile) {
    std::map<RuleType, Rule> rules;
    
    auto json = Core::readJsonFile(rulesFile);
    
    if(json["rules"].is_array())
    {
      auto jsonRules = json["rules"].get<std::vector<nlohmann::json>>();
      
      for(const auto& jsonRule : jsonRules)
      {
        const std::string parameter = (jsonRule.find("parameter") != jsonRule.end()) ? jsonRule["parameter"].get<std::string>() : "";
        const std::string appliedTo = (jsonRule.find("appliedTo") != jsonRule.end()) ? jsonRule["appliedTo"].get<std::string>() : "All";
        Rule rule(Core::ScopeType_to_enum_class(appliedTo),
                  RuleType_to_enum_class(jsonRule["rule"].get<std::string>()),
                  parameter);
        rules[rule.getRuleType()] = rule;
      }
    }
    
    return rules;
  }
  
  std::ostream& operator<<(std::ostream& out, const Syntax::Rule& rule) {
    out << "Rule: " << to_string(rule.getRuleType()) << " - Applied To: " << Core::to_string(rule.getScopeType());
    if(rule.hasParameter()) {
      out << " - Parameter: " << rule.getParameter();
    }
    return out;
  }
  
  Rule::Rule(Core::ScopeType applyTo, RuleType type, const std::string& optionalParameter)
    : m_applyTo(applyTo)
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

  bool Rule::hasParameter() const {
    return m_parameter.size() > 0;
  }

  const std::string& Rule::getParameter() const {
    return m_parameter;
  }

}
