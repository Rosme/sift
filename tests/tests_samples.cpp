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

#include "core/utils.hpp"
#include "syntax/rule.hpp"

TEST_CASE("Testing 1TBS syntax style", "[sample]") {

  std::string ruleFilePath = "samples/rules/1TBS.json";
  std::string sourceFilePath = "samples/src/brightness_manager_1TBS.cc";
  Core::File sourceFile = new Core::File();
  std::vector<Rule> rules;

  SECTION("Reading rules from json file") {
	rules = Syntax::readRules(ruleFilePath);

	REQUIRE(rules.size() == 8);
  }
  
  SECTION("Reading source file") {
    bool success = Core::readSourceFile(sourceFilePath, sourceFile);
    
    REQUIRE(success);
  }
  
  SECTION("Verifying source file syntax") {
    //TODO
  }
  
}

TEST_CASE("Testing Google syntax style", "[sample]") {

	std::string ruleFilePath = "samples/rules/Google.json";
	std::string sourceFilePath = "samples/src/brightness_manager_Google.cc";
	Core::File sourceFile = new Core::File();
	std::vector<Rule> rules;

	SECTION("Reading rules from json file") {
		rules = Syntax::readRules(ruleFilePath);

		REQUIRE(rules.size() == 11);
	}

	SECTION("Reading source file") {
		bool success = Core::readSourceFile(sourceFilePath, sourceFile);

		REQUIRE(success);
	}

	SECTION("Verifying source file syntax") {
		//TODO
	}

}

TEST_CASE("Testing K&R syntax style", "[sample]") {

	std::string ruleFilePath = "samples/rules/KnR.json";
	std::string sourceFilePath = "samples/src/brightness_manager_KnR.cc";
	Core::File sourceFile = new Core::File();
	std::vector<Rule> rules;

	SECTION("Reading rules from json file") {
		rules = Syntax::readRules(ruleFilePath);

		REQUIRE(rules.size() == 7);
	}

	SECTION("Reading source file") {
		bool success = Core::readSourceFile(sourceFilePath, sourceFile);

		REQUIRE(success);
	}

	SECTION("Verifying source file syntax") {
		//TODO
	}

}

TEST_CASE("Testing Allman syntax style", "[sample]") {

	std::string ruleFilePath = "samples/rules/Allman.json";
	std::string sourceFilePath = "samples/src/brightness_manager_Allman.cc";
	Core::File sourceFile = new Core::File();
	std::vector<Rule> rules;

	SECTION("Reading rules from json file") {
		rules = Syntax::readRules(ruleFilePath);

		REQUIRE(rules.size() == 7);
	}

	SECTION("Reading source file") {
		bool success = Core::readSourceFile(sourceFilePath, sourceFile);

		REQUIRE(success);
	}

	SECTION("Verifying source file syntax") {
		//TODO
	}

}

TEST_CASE("Testing GNU syntax style", "[sample]") {

	std::string ruleFilePath = "samples/rules/GNU.json";
	std::string sourceFilePath = "samples/src/brightness_manager_GNU.cc";
	Core::File sourceFile = new Core::File();
	std::vector<Rule> rules;

	SECTION("Reading rules from json file") {
		rules = Syntax::readRules(ruleFilePath);

		REQUIRE(rules.size() == 6);
	}

	SECTION("Reading source file") {
		bool success = Core::readSourceFile(sourceFilePath, sourceFile);

		REQUIRE(success);
	}

	SECTION("Verifying source file syntax") {
		//TODO
	}

}

TEST_CASE("Testing Whitesmiths syntax style", "[sample]") {

	std::string ruleFilePath = "samples/rules/Whitesmiths.json";
	std::string sourceFilePath = "samples/src/brightness_manager_Whitesmiths.cc";
	Core::File sourceFile = new Core::File();
	std::vector<Rule> rules;

	SECTION("Reading rules from json file") {
		rules = Syntax::readRules(ruleFilePath);

		REQUIRE(rules.size() == 7);
	}

	SECTION("Reading source file") {
		bool success = Core::readSourceFile(sourceFilePath, sourceFile);

		REQUIRE(success);
	}

	SECTION("Verifying source file syntax") {
		//TODO
	}

}