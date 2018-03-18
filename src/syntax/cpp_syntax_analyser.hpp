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

#include <map>
#include <functional>

#include "syntax_analyser.hpp"
#include "rule.hpp"

#include "../core/scope.hpp"
#include "../core/message_stack.hpp"

namespace Syntax {
  
  class CPPSyntaxAnalyser : public SyntaxAnalyser {
  public:
    CPPSyntaxAnalyser();
    virtual ~CPPSyntaxAnalyser();
    
    std::string getRuleMessage(const Syntax::Rule& rule);
    void registerRuleWork(std::map<Syntax::RuleType, std::function<void(Syntax::Rule&, Core::Scope&, Core::MessageStack&)>>& work,
                          const std::map<std::string, std::vector<Core::Scope>>& literals = std::map<std::string, std::vector<Core::Scope>>(),
                          const std::map<std::string, std::vector<Core::Scope>>& comments = std::map<std::string, std::vector<Core::Scope>>());
    
    void RuleUnknown(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoAuto(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoDefine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoMacroFunctions(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleStartWithX(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleEndWithX(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleMaxCharactersPerLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleCurlyBracketsOpenSameLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleCurlyBracketsOpenSeparateLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleCurlyBracketsCloseSameLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleCurlyBracketsCloseSeparateLine(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleAlwaysHaveCurlyBrackets(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoConstCast(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleStartWithLowerCase(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleStartWithUpperCase(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNameMaxCharacter(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleSingleReturn(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoGoto(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleSpaceBetweenOperandsInternal(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoSpaceBetweenOperandsInternal(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoCodeAllowedSameLineCurlyBracketsOpen(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleNoCodeAllowedSameLineCurlyBracketsClose(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleTabIndentation(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleCurlyBracketsIndentationAlignWithDeclaration(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleElseSeparateLineFromCurlyBracketClose(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleOwnHeaderBeforeStandard(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);
    void RuleStandardHeaderBeforeOwn(Syntax::Rule& rule, Core::Scope& rootScope, Core::MessageStack& messageStack);

  private:
    bool isScopeUsingCurlyBrackets(Core::Scope& scope);
    bool isOpeningCurlyBracketSeparateLine(Core::Scope& scope);
    bool isClosingCurlyBracketSeparateLine(Core::Scope& scope);
    bool checkSpaceBetweenOperandsInternal(Core::Scope& scope, bool noSpace);
    bool noCodeAfterCurlyBracketSameLineOpen(Core::Scope& scope);
    bool noCodeAfterCurlyBracketSameLineClose(Core::Scope& scope);
    
    void pushErrorMessage(Core::MessageStack& messageStack, Syntax::Rule& rule, const std::string& line, const Core::Scope& scope);
    Core::ScopeType computeApplicableScopeTypes(Core::ScopeType input, Core::ScopeType defaultAll, Core::ScopeType ignoredTypes);
    std::vector<Core::Scope> getComments(const std::string& filename) const;
    bool isWithinComment(unsigned int line, unsigned int position, Core::File& file);
    std::vector<Core::Scope> getStringLiterals(const std::string& filename) const;
    bool isWithinStringLiteral(unsigned int line, unsigned int position, Core::File& file);

    bool validateOwnHeaderBeforeStandard(const std::string& header, bool& hasSeenStandard);
    bool validateStandardHeaderBeforeOwn(const std::string& header, bool& hasSeenOwn);
    
    // Combines comment + string literal checking
    bool isWithinIgnoredScope(unsigned int line, unsigned int position, Core::File& file);

    const std::map<std::string, std::vector<Core::Scope>>* m_stringLiterals = nullptr;
    const std::map<std::string, std::vector<Core::Scope>>* m_comments = nullptr;
  };
  
}
