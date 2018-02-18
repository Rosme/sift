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
#include "pfe.hpp"

#define DUMP_STACK(stack) for(auto&& m : stack.getMessages()) std::cout << m << std::endl;

std::vector<char*> convert(std::vector<std::string>& base)
{
  std::vector<char*> argv;
  for (const auto& arg : base)
    argv.push_back((char*)arg.data());
  argv.push_back(nullptr);
  return argv;
}

Core::MessageStack doTest(PFE& pfe, const std::string rules, const std::string source)
{
  pfe.setupRules(rules);
  pfe.registerRuleWork();
  pfe.readPath(source);
  pfe.extractScopes();
  pfe.applyRules();
  return pfe.getMessageStack();
}

TEST_CASE("Testing macro related rules", "[rules-macro]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();
  
  SECTION("Test No defines") {
    const auto stack = doTest(pfe, "samples/tests/rules/nodefines.json", "samples/src/define.hpp");
    REQUIRE(stack.size() == 8);
  }
  
  SECTION("Test No macro functions") {
    const auto stack = doTest(pfe, "samples/tests/rules/nomacrofunctions.json", "samples/src/define.hpp");
    REQUIRE(stack.size() == 4);
  }
}

TEST_CASE("Testing suffix/prefix related rules", "[rules-suffix]") {
  std::vector<std::string> argv = {"program_name", "-V"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging(); 
  
  SECTION("Test Prefix") {
    const auto stack = doTest(pfe, "samples/tests/rules/prefix.json", "samples/src/prefix.cpp");
    DUMP_STACK(stack);
    REQUIRE(stack.size() == 6);
  }
}

TEST_CASE("Testing max characters per line", "[rules-maxcharperline]") {
  std::vector<std::string> argv = {"program_name", "-V"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test Maximum Characters Per Line") {
    const auto stack = doTest(pfe, "samples/tests/rules/maxcharactersperline.json", "samples/tests/src/maxcharactersperline.cpp");
    DUMP_STACK(stack);
    REQUIRE(stack.size() == 3);
  }
}

TEST_CASE("Testing No Const Cast", "[rules-noconstcast]") {
  std::vector<std::string> argv = {"program_name", "-V"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test Finding Const CasT Not In Comments") {
    const auto stack = doTest(pfe, "samples/tests/rules/noconstcast.json", "samples/tests/src/noconstcast.cpp");
    DUMP_STACK(stack);
    REQUIRE(stack.size() == 1);
  }
}