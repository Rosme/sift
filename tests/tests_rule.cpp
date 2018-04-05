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
#include "syntax/rule.hpp"

Syntax::Rule RULE(RuleId ruleId, Syntax::RuleType rule, Core::ScopeType appliedTo, std::string parameter = ""){
  return Syntax::Rule(ruleId, appliedTo, rule, parameter);
}

Syntax::Rule RULE(RuleId ruleId, Syntax::RuleType rule){
  return RULE(ruleId, rule, Core::ScopeType::All, "");
}

TEST_CASE("Testing macro related rules", "[rules-macro]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();
  
  SECTION("Test No defines") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/nodefines.json", "samples/src/define.hpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 8);
  }
  
  SECTION("Test No macro functions") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/nomacrofunctions.json", "samples/src/define.hpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing suffix/prefix related rules", "[rules-suffix]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();
  
  SECTION("Test Prefix") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/prefix.json", "samples/src/prefix.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 11);
  }
}

TEST_CASE("Testing max characters per line", "[rules-maxcharperline]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test Maximum Characters Per Line") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/maxcharactersperline.json", "samples/tests/src/maxcharactersperline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 3);
  }

  SECTION("Test Maximum Characters Per Line") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/maxcharactersperlinenoparam.json", "samples/tests/src/maxcharactersperline.cpp");
    REQUIRE(stack.size() == 0);
  }
}

TEST_CASE("Testing No Const Cast", "[rules-noconstcast]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test Finding Const CasT Not In Comments") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/noconstcast.json", "samples/tests/src/noconstcast.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
}

TEST_CASE("Testing Max Character For a Name", "[rules-maxcharpername]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test Max Character For All") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/namemaxcharacterall.json", "samples/tests/src/namemaxcharacter.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }

  SECTION("Test Max Character For Variables Only") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/namemaxcharactersvariableonly.json", "samples/tests/src/namemaxcharacter.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }

  SECTION("Test Max Character For Variables Only") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/namemaxcharacternoparam.json", "samples/tests/src/namemaxcharacter.cpp");
    REQUIRE(stack.size() == 0);
  }
}

TEST_CASE("Testing curly brackets on same or separate line", "[rules-curlybracketline]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test opening curly brackets on same line") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/curlybracketsopensameline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 6);
  }

  SECTION("Test opening curly brackets on separate line") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/curlybracketsopenseparateline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }

  SECTION("Test closing curly brackets on same line") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/curlybracketsclosesameline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
      REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 6);
  }

  SECTION("Test closing curly brackets on separate line") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/curlybracketscloseseparateline.json", "samples/tests/src/curlybracketsameorseparateline.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing start with upper/lower case", "[rules-upperlower]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test start with lower case") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/startwithlowercase.json", "samples/tests/src/startwithloweruppercase.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 3);
  }

  SECTION("Test start with upper case") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/startwithuppercase.json", "samples/tests/src/startwithloweruppercase.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing always have curly brackets", "[rules-alwayscurlybrackets]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test always have curly brackets") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/alwayshavecurlybrackets.json", "samples/tests/src/alwayshavecurlybrackets.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }
}

TEST_CASE("Testing single return", "[rules-singlereturn]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test single return") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/singlereturn.json", "samples/tests/src/singlereturn.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
}

TEST_CASE("Testing no auto", "[rules-noauto]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test no auto") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/noauto.json", "samples/tests/src/noauto.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 2);
  }
  
  
  SECTION("Auto within comments") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::NoAuto)}
    };
    std::vector<std::string> source = {
      "  #define some_define(x) /* this is auto */ ",
      "\tint main(){",
      "  int x = 3; /* auto does not mean bongo */",
      "\t }",
    };
    
    const auto stack = doTestWithSource(sift, ruleMap, source);
    REQUIRE(stack.size() == 0);
  }
  
  SECTION("Auto within literals") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::NoAuto)}
    };
    std::vector<std::string> source = {
      "\tint main(){",
      " std::string literal = \"auto is a literal\"",
      "\t }",
    };
    
    const auto stack = doTestWithSource(sift, ruleMap, source);
    REQUIRE(stack.size() == 0);
  }
}

TEST_CASE("Testing no goto", "[rules-nogoto]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Against file") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/nogoto.json", "samples/tests/src/nogoto.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
  
  const auto ruleId = 0;
  std::map<RuleId, Syntax::Rule> ruleMap = {
    {ruleId, RULE(ruleId, Syntax::RuleType::NoGoto)}
  };
  
  SECTION("Find in function") {
    const auto stack = doTestWithSource(sift, ruleMap, "int main(){ goto label; }");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
  
  SECTION("Ignore partial wording") {
    const auto stack = doTestWithSource(sift, ruleMap, "int main(){ auto autogoto = \"string\"; }");
    REQUIRE(stack.size() == 0);
  }
}

TEST_CASE("Testing space between operands internal", "[rules-spacebetweenoperandsinternal]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Test space between operands internal") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/spacebetweenoperandsinternal.json", "samples/tests/src/spacebetweenoperandsinternal.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 4);
  }

  SECTION("Test no space between operands internal") {
    const auto stack = doTestWithFile(sift, "samples/tests/rules/nospacebetweenoperandsinternal.json", "samples/tests/src/spacebetweenoperandsinternal.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 3);
  }
}

TEST_CASE("Testing no code allowed same line curly brackets", "[rules-nocodeallowedsamelinecurlybrackets]") {
  std::vector<std::string> argv = { "program_name", "-q" };
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  //SECTION("Test no code allowed same line opening curly brackets") {
  //  const auto stack = doTestWithFile(sift, "samples/tests/rules/nocodeallowedsamelinecurlybracketsopen.json", "samples/tests/src/nocodeallowedsamelinecurlybrackets.cpp");
  //  REQUIRE(stack.size() == 1);
  //  REQUIRE(stack.getMessages().begin()->second.size() == 3);
  //}

  //SECTION("Test no code allowed same line closing curly brackets") {
  //  const auto stack = doTestWithFile(sift, "samples/tests/rules/nocodeallowedsamelinecurlybracketsclose.json", "samples/tests/src/nocodeallowedsamelinecurlybrackets.cpp");
  //  REQUIRE(stack.size() == 1);
  //  REQUIRE(stack.getMessages().begin()->second.size() == 2);
  //}
}

TEST_CASE("Testing TabIndentation", "[rules-tabindentation]") {
  std::vector<std::string> argv = { "program_name", "-q" };

  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();
  
  SECTION("All green") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::TabIndentation)}
    };
    
    // By definition, the 1TBS style should have tab indentation
    const auto stack = doTestWithFile(sift, ruleMap, "samples/src/brightness_manager_1TBS.cc");
    REQUIRE(stack.size() == 0);
  }
  
  SECTION("Against Google") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::TabIndentation)}
    };
    
    const auto stack = doTestWithFile(sift, ruleMap, "samples/src/brightness_manager_Google.cc");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 77);
  }
  
  SECTION("Two errors") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::TabIndentation)}
    };
    std::vector<std::string> source = {
      "\tHello",
      "  spaces are evil",
      "\t this is wrong still",
    };
    
    const auto stack = doTestWithSource(sift, ruleMap, source);
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 2);
  }
  
  SECTION("Partially tabbed function") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::TabIndentation)}
    };
    std::vector<std::string> source = {
      "\tint main(){",
      "  std::cout << \"This doesn't compile anyway\" << std::endl;",
      " ", // This line shouldn't count as wrong
      "\t return 0;"
      "\t }",
    };
    
    const auto stack = doTestWithSource(sift, ruleMap, source);
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 3);
  }
}

TEST_CASE("Testing Curly Brackets Alignement", "[rules-bracketsalignment]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Curly Brackets Indentation Align With Declaration For All") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::CurlyBracketsIndentationAlignWithDeclaration)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/curlybracketsindentationalignwithdeclaration.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 7);
  }

  SECTION("Curly Brackets Indentation Align With Declaration For Class Only") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::CurlyBracketsIndentationAlignWithDeclaration, Core::ScopeType::Class)}
    };
    
    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/curlybracketsindentationalignwithdeclaration.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
}

TEST_CASE("Testing Conditional And Line Curly Bracket", "[rules-conditionallinecurlybrackets]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Else On Seperate Line From Curly Bracket Close") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::ElseSeparateLineFromCurlyBracketClose)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/elseseperatelinefromcurlybracketclose.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 1);
  }
}

TEST_CASE("Testing Header Order", "[rules-headerorder]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Own Header Before Standard - Standard Only") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::OwnHeaderBeforeStandard)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/headerorderstandardalone.cpp");
    REQUIRE(stack.size() == 0);
  }

  SECTION("Own Header Before Standard - Own Only") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::OwnHeaderBeforeStandard)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/headerorderownalone.cpp");
    REQUIRE(stack.size() == 0);
  }

  SECTION("Own Header Before Standard - Good") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::OwnHeaderBeforeStandard)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/ownheaderbeforestandardgood.cpp");
    REQUIRE(stack.size() == 0);
  }

  SECTION("Own Header Before Standard - Bad") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::OwnHeaderBeforeStandard)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/ownheaderbeforestandardbad.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 2);
  }

  SECTION("Standard Header Before Own - Standard Only") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::StandardHeaderBeforeOwn)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/headerorderstandardalone.cpp");
    REQUIRE(stack.size() == 0);
  }

  SECTION("Standard Header Before Own - Own Only") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::StandardHeaderBeforeOwn)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/headerorderownalone.cpp");
    REQUIRE(stack.size() == 0);
  }

  SECTION("Standard Header Before Own - Good") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::StandardHeaderBeforeOwn)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/standardheaderbeforeowngood.cpp");
    REQUIRE(stack.size() == 0);
  }

  SECTION("Standard Header Before Own - Bad") {
    const auto ruleId = 0;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleId, RULE(ruleId, Syntax::RuleType::StandardHeaderBeforeOwn)}
    };

    const auto stack = doTestWithFile(sift, ruleMap, "samples/tests/src/standardheaderbeforeownbad.cpp");
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.getMessages().begin()->second.size() == 2);
  }
}

TEST_CASE("Rule Appliance", "[rules-appliance]") {
  std::vector<std::string> argv = {"program_name", "-q"};
  SIFT sift;
  sift.parseArgv(argv.size(), convert(argv).data());
  sift.setupLogging();

  SECTION("Same rule on two different scope") {
    const auto ruleOne = 0;
    const auto ruleTwo = 1;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleOne, RULE(ruleOne, Syntax::RuleType::StartWithX, Core::ScopeType::Class, "c_")},
      {ruleTwo, RULE(ruleTwo, Syntax::RuleType::StartWithX, Core::ScopeType::ClassVariable, "m_")}
    };
    std::vector<std::string> source = {
      "int itsFine = 1;",
      "class ErrorClass {",
      "  int m_itsOkay = 2;",
      "  int errorVariable = 3;",
      "};",
      "class c_OkayClass {",
      "};"
    };

    const auto stack = doTestWithSource(sift, ruleMap, source);
    const auto& messages = stack.getMessages();
    REQUIRE(stack.size() == 2);
    REQUIRE(messages.at(ruleOne).size() == 1);
    REQUIRE(messages.at(ruleOne).at(0).line == 1);
    REQUIRE(messages.at(ruleTwo).size() == 1);
    REQUIRE(messages.at(ruleTwo).at(0).line == 3);
  }

  SECTION("Same rule definition twice") {
    const auto ruleOne = 0;
    const auto ruleTwo = 1;
    std::map<RuleId, Syntax::Rule> ruleMap = {
      {ruleOne, RULE(ruleOne, Syntax::RuleType::StartWithX, Core::ScopeType::Class, "c_")},
      {ruleTwo, RULE(ruleTwo, Syntax::RuleType::StartWithX, Core::ScopeType::Class, "c_")}
    };

    std::vector<std::string> source = {
      "int itsFine = 1;",
      "class ErrorClass {",
      "  int m_itsOkay = 2;",
      "  int errorVariable = 3;",
      "};",
      "class c_OkayClass {",
      "};"
    };

    const auto stack = doTestWithSource(sift, ruleMap, source);
    const auto& messages = stack.getMessages();
    REQUIRE(stack.size() == 1);
    REQUIRE(messages.at(ruleOne).size() == 1);
    REQUIRE(messages.at(ruleOne).at(0).line == 1);
  }
}
  
TEST_CASE("Rule conflicts", "[rules-conflicts]") {
    std::vector<std::string> argv = {"program_name", "-q"};
    SIFT sift;
    sift.parseArgv(argv.size(), convert(argv).data());
    sift.setupLogging();
  SECTION("Rule with conflict on 'all' (same, verbatim) scopes") {

    sift.clearState();
    auto x = "samples/tests/rules/conflictclashsamescopes.json";
    sift.setupRules(x);
    sift.registerRuleWork();

    const auto& rules = sift.getRules();
    REQUIRE(rules.size() == 1);
    
  }
  
  SECTION("Rule with conflict on encompassing scopes") {
    
    sift.clearState();
    auto x = "samples/tests/rules/conflictclashdifferentscopes.json";
    sift.setupRules(x);
    sift.registerRuleWork();
    
    const auto& rules = sift.getRules();
    REQUIRE(rules.size() == 1);
    
  }
  
  SECTION("Rule with no conflict because non-encompassing scopes") {
    
    sift.clearState();
    auto x = "samples/tests/rules/conflictnoclash.json";
    sift.setupRules(x);
    sift.registerRuleWork();
    
    const auto& rules = sift.getRules();
    REQUIRE(rules.size() == 2);
    
  }
}
