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

#define DUMP_STACK(stack) for(auto&& mp : stack.getMessages()){ \
  std::cout << "RuleId: " << mp.first << ", " << pfe.getRules().at(mp.first) << std::endl; \
  for(auto&& m : mp.second) \
  { \
    std::cout << "\t" << m << std::endl;\
  } \
}

std::vector<char*> convert(std::vector<std::string>& base)
{
  std::vector<char*> argv;
  for (const auto& arg : base)
    argv.push_back((char*)arg.data());
  argv.push_back(nullptr);
  return argv;
}

const Core::MessageStack doTest(PFE& pfe, const std::string rules, const std::string source)
{
  pfe.setupRules(rules);
  pfe.registerRuleWork();
  pfe.readPath(source);
  pfe.extractScopes();
  pfe.applyRules();
  return pfe.getMessageStacks().at(source);
}

TEST_CASE("Testing macro related rules", "[rules-macro]") {
  std::vector<std::string> argv = {"program_name", ""};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();
  
  SECTION("Test No defines") {
    const auto stack = doTest(pfe, "samples/tests/rules/nodefines.json", "samples/src/define.hpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 8);
  }
  
  SECTION("Test No macro functions") {
    const auto stack = doTest(pfe, "samples/tests/rules/nomacrofunctions.json", "samples/src/define.hpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing suffix/prefix related rules", "[rules-suffix]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging(); 
  
  SECTION("Test Prefix") {
    const auto stack = doTest(pfe, "samples/tests/rules/prefix.json", "samples/src/prefix.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 10);
  }
}

TEST_CASE("Testing max characters per line", "[rules-maxcharperline]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test Maximum Characters Per Line") {
    const auto stack = doTest(pfe, "samples/tests/rules/maxcharactersperline.json", "samples/tests/src/maxcharactersperline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 3);
  }

  SECTION("Test Maximum Characters Per Line") {
    const auto stack = doTest(pfe, "samples/tests/rules/maxcharactersperlinenoparam.json", "samples/tests/src/maxcharactersperline.cpp");
    REQUIRE(stack.size() == 0);
  }
}

TEST_CASE("Testing No Const Cast", "[rules-noconstcast]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test Finding Const CasT Not In Comments") {
    const auto stack = doTest(pfe, "samples/tests/rules/noconstcast.json", "samples/tests/src/noconstcast.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
}

TEST_CASE("Testing Max Character For a Name", "[rules-maxcharpername]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test Max Character For All") {
    const auto stack = doTest(pfe, "samples/tests/rules/namemaxcharacterall.json", "samples/tests/src/namemaxcharacter.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }

  SECTION("Test Max Character For Variables Only") {
    const auto stack = doTest(pfe, "samples/tests/rules/namemaxcharactersvariableonly.json", "samples/tests/src/namemaxcharacter.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }

  SECTION("Test Max Character For Variables Only") {
    const auto stack = doTest(pfe, "samples/tests/rules/namemaxcharacternoparam.json", "samples/tests/src/namemaxcharacter.cpp");
    REQUIRE(stack.size() == 0);
  }
}

TEST_CASE("Testing curly brackets on same or separate line", "[rules-curlybracketline]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test opening curly brackets on same line") {
    const auto stack = doTest(pfe, "samples/tests/rules/curlybracketsopensameline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 6);
  }

  SECTION("Test opening curly brackets on separate line") {
    const auto stack = doTest(pfe, "samples/tests/rules/curlybracketsopenseparateline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }

  SECTION("Test closing curly brackets on same line") {
    const auto stack = doTest(pfe, "samples/tests/rules/curlybracketsclosesameline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
      REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 6);
  }

  SECTION("Test closing curly brackets on separate line") {
    const auto stack = doTest(pfe, "samples/tests/rules/curlybracketscloseseparateline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing start with upper/lower case", "[rules-upperlower]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test start with lower case") {
    const auto stack = doTest(pfe, "samples/tests/rules/startwithlowercase.json", "samples/tests/src/startwithloweruppercase.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 3);
  }

  SECTION("Test start with upper case") {
    const auto stack = doTest(pfe, "samples/tests/rules/startwithuppercase.json", "samples/tests/src/startwithloweruppercase.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing always have curly brackets", "[rules-alwayscurlybrackets]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test always have curly brackets") {
    const auto stack = doTest(pfe, "samples/tests/rules/alwayshavecurlybrackets.json", "samples/tests/src/alwayshavecurlybrackets.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing single return", "[rules-singlereturn]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test single return") {
    const auto stack = doTest(pfe, "samples/tests/rules/singlereturn.json", "samples/tests/src/singlereturn.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
}

TEST_CASE("Testing no auto", "[rules-noauto]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test no auto") {
    const auto stack = doTest(pfe, "samples/tests/rules/noauto.json", "samples/tests/src/noauto.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 2);
  }
}

TEST_CASE("Testing no goto", "[rules-nogoto]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  PFE pfe;
  pfe.parseArgv(argv.size(), convert(argv).data());
  pfe.setupLogging();

  SECTION("Test no goto") {
    const auto stack = doTest(pfe, "samples/tests/rules/nogoto.json", "samples/tests/src/nogoto.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
}
