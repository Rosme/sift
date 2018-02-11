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

#include "core/message_stack.hpp"
#include "core/file.hpp"
#include "core/scope.hpp"
#include "syntax/rule.hpp"
#include "syntax/syntax_analyser.hpp"
#include "flow/flow_analyser.hpp"

namespace Core
{
  // Class to render the function flow explicit, better than to have everything laid out in main
  class PFE
  {
  public:
    PFE();
    
    // Move these to syntax_analyzer accordingly
    void parseArgv(int argc, char** argv);
    void setupLogging();
    void setupRules(const std::string filename);
    void readSingleSourceFile(const std::string& filename);
    void readFilesFromDirectory(const std::string& directory);
    void extractScopes();
    void applyRules();
    void registerRuleWork();
    void outputMessages();
    
    std::map<std::string, Core::Scope> getScopes() { return m_rootScopes; }
  private:
    bool extractScopesFromFile(Core::File& file, Core::Scope &outScope);
    void extractDefines(Core::File& file, Core::Scope &outScope);
    void extractNamespaces(Core::File& file, Core::Scope& parent);
    void extractEnums(Core::File& file, Core::Scope& parent);
    void extractConditionals(Core::File& file, Core::Scope& parent);

    void constructTree(Core::Scope& root);
    Core::Scope& findBestParent(Core::Scope& root, Core::Scope& toSearch);
    
    int findEndOfScope(Core::Scope& scope, Core::File& file, int startingLine);
    int findEndOfScope(Core::Scope& scope, Core::File& file, int startingLine, int startingCharacter); 
    int findEndOfScopeConditionalFor(Core::Scope& scope, Core::File& file, int startingLine, int startingCharacter);
    int findEndOfScopeConditionalDoWhile(Core::Scope& scope, Core::File& file, int startingLine);

    bool isDoWhileLoop(Core::File& file, int startingLine, int startingCharacter);

    // filename : rawText
    std::map<std::string, Core::File> m_files;
    
    // filename : rootScope
    std::map<std::string, Core::Scope> m_rootScopes;
    
    // ruleType : rule
    std::map<Syntax::RuleType, Syntax::Rule> m_rules;
    
    // ruleType : work
    std::map<Syntax::RuleType, std::function<void(Syntax::Rule&, Core::Scope&, Core::MessageStack&)>> m_rulesWork;
    
    std::unique_ptr<Flow::FlowAnalyser> m_flowAnalyser;
    std::unique_ptr<Syntax::SyntaxAnalyser> m_syntaxAnalyser;
    Core::MessageStack m_messageStack;
    
    bool m_quietMode = false;
    std::string m_outputFilename;
    std::string m_loggingSettingsFilename;
  };
};
