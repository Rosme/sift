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

#include "catch.hh"
#include "utils.hpp"
#include "flow/cpp_flow_analyser.hpp"
#include "core/message_stack.hpp"

TEST_CASE("Testing null pointer", "[rules-nullpointer]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();
  Flow::CPPFlowAnalyser flowAnalyser = Flow::CPPFlowAnalyser();

  SECTION("Two errors") {

    std::vector<std::string> source = {
      "int main()",
      "{",
      "  int *firstPointer;",
      "  int *secondPointer = 0;",
      "  int *thirdPointer = NULL;",
      "  firstPointer = NULL;",
      " cout << firstPointer << endl;",
      " cout << secondPointer << endl;",
      "}"
    };

    sift.clearState();
    sift.readSource("dummy_filename", source);
    sift.extractScopes();
    std::map<const std::string, Core::MessageStack> messageStacksFlow;

    for (auto& scopePair : sift.getScopes()) {
      flowAnalyser.analyzeNullPointer(scopePair.second, messageStacksFlow[scopePair.second.file->filename]);
    }

    const auto stack = messageStacksFlow.at("dummy_filename");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 2);
  }
}

TEST_CASE("Testing uninitialized variable", "[rules-Uninitializedvariable]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();
  Flow::CPPFlowAnalyser flowAnalyser = Flow::CPPFlowAnalyser();

  SECTION("Two errors") {

    std::vector<std::string> source = {
      "int main()",
      "{",
      "  int firstvar;",
      "  int secondvar = 3;",
      "  int thirdvar;",
      "  int fourthvar;",
      "  thirdvar = secondvar;",
      " cout << firstvar << endl;",
      " cout << secondvar << endl;",
      " cout << thirdvar << endl;",
      "  thirdvar = fourthvar;",
      "}"
    };

    sift.clearState();
    sift.readSource("dummy_filename", source);
    sift.extractScopes();

    std::map<const std::string, Core::MessageStack> messageStacksFlow;

    for (auto& scopePair : sift.getScopes()) {
      flowAnalyser.analyzeUninitializedVariable(scopePair.second, messageStacksFlow[scopePair.second.file->filename]);
    }

    const auto stack = messageStacksFlow.at("dummy_filename");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 2);
  }
}