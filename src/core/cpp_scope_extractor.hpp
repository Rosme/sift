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
#include <map>

#include "scope_extractor.hpp"

namespace Core {

  class CppScopeExtractor : public ScopeExtractor {
  public:
    bool extractScopesFromFile(File& file, Scope &outScope);
    void constructTree(Scope& root);

  private:
    void extractDefines(File& file, Scope &parent);
    void extractNamespaces(File& file, Scope& parent);
    void extractEnums(File& file, Scope& parent);
    void extractClasses(File& file, Scope& parent);
    void extractFunctions(File& file, Scope& parent);
    void extractVariables(File& file, Scope& parent);
    void extractConditionals(File& file, Scope& parent);
    void extractComments(File& file, Scope& parent);
    void extractStringLiterals(File& file, Scope& parent);

    void filterScopes(Scope& root);
    Scope& findBestParent(Scope& root, Scope& toSearch);

    int findEndOfScope(Scope& scope, File& file, int startingLine);
    int findEndOfScope(Scope& scope, File& file, int startingLine, int startingCharacter);
    int findEndOfScopeConditionalFor(Scope& scope, File& file, int startingLine, int startingCharacter);
    int findEndOfScopeConditionalDoWhile(Scope& scope, File& file, int startingLine);
    bool isDoWhileLoop(File& file, int startingLine, int startingCharacter);
    bool isWithinStringLiteral(unsigned int pos, unsigned int line, File& file) const;
    bool isWithinComment(unsigned int pos, unsigned int line, File& file, Scope& parent) const; 
    bool isLineWithinDefine(const std::string& filename, int line);

    static const std::vector<std::string> ReservedKeywords;
    std::map<std::string, std::vector<Core::Scope>> m_defineScopes;
  };

}
