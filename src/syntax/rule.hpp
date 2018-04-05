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

#include <vector>
#include <string>

#include <rosme/smartenum.hpp>

#include "../core/constants.hpp"
#include "../core/scope.hpp"

namespace Syntax {
  
  /* RULE_DEFINE_ENUM */
  smart_enum_class(RuleType,
                   Unknown = 0,
                   NoAuto,
                   NoDefine,
                   NoMacroFunctions,
                   StartWithX,
                   EndWithX,
                   MaxCharactersPerLine,
                   CurlyBracketsOpenSameLine,
                   CurlyBracketsOpenSeparateLine,
                   CurlyBracketsCloseSameLine,
                   CurlyBracketsCloseSeparateLine,
                   AlwaysHaveCurlyBrackets,
                   NoConstCast,
                   StartWithLowerCase,
                   StartWithUpperCase,
                   NameMaxCharacter,
                   SingleReturn,
                   NoGoto,
                   SpaceBetweenOperandsInternal,
                   NoSpaceBetweenOperandsInternal,
                   NoCodeAllowedSameLineCurlyBracketsOpen,
                   NoCodeAllowedSameLineCurlyBracketsClose,
                   TabIndentation,
                   CurlyBracketsIndentationAlignWithDeclaration,
                   ElseSeparateLineFromCurlyBracketClose,
                   OwnHeaderBeforeStandard,
                   StandardHeaderBeforeOwn
                  );
  /* END_RULE_DEFINE_ENUM */
  
  class Rule {
  public:
    Rule() { }
    Rule(RuleId id, Core::ScopeType applyTo, RuleType type, const std::string& optionalParameter = "");
    
    Core::ScopeType getScopeType() const;
    RuleType getRuleType() const;
    RuleId getRuleId() const;
    bool hasParameter() const;
    const std::string& getParameter() const;

  private:
    RuleId m_id;
    Core::ScopeType m_applyTo;
    RuleType m_type;
    std::string m_parameter;
  };

  std::ostream& operator<<(std::ostream& out, const Syntax::Rule& rule);
  std::map<RuleId, Rule> readRules(const std::map<Syntax::RuleType, std::vector<Syntax::RuleType>> &conflicts, const std::string& rulesFile);
  bool operator==(const Rule& lhs, const Rule& rhs);
}
