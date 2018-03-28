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

#include "flow_analyser.hpp"

#include "../core/scope.hpp"
#include "../core/message_stack.hpp"

namespace Flow {
  
  class CPPFlowAnalyser : public FlowAnalyser {
  public:
    CPPFlowAnalyser();
    virtual ~CPPFlowAnalyser();

    void analyzeFlow(Core::Scope& rootScope, Core::MessageStack& messageStack);
    void analyzeNullPointer(Core::Scope& rootScope, Core::MessageStack& messageStack);
    void analyzeUninitializedVariable(Core::Scope& rootScope, Core::MessageStack& messageStack);

  private:

    int scopeUsingNullPointer(Core::Scope& scope);
    int scopeUsingUninitializedVariable(Core::Scope& scope);
    bool isScopeVariablePrimitive(Core::Scope& scope);
    bool isVariableValueChanged(const std::string& line, bool isVariableDeclarationLine, int positionAfterVarName, std::string& varValue);
    bool isVariableValueValid(std::string varValue, bool isPointer);
  };
  
}
