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

#include "core/config.hpp"
#include "core/assert.hpp"

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <fstream>

#ifdef WIN32
  #define BIN_NAME "sift.exe"
#else
  #define BIN_NAME "./sift"
#endif

#define TESTS_DIR "temp-system-tests/"

#define DUMP_COLLECTION(x) \
for(const auto& item : x) {\
  std::cout << "  " << item << std::endl;\
}

#define REQUIRE_OUTPUT_SIZE(collection, target)\
if(collection.size() != target){\
  std::cout << "Command output:" << std::endl;\
  DUMP_COLLECTION(collection);\
} \
REQUIRE(collection.size() == target);

static int currentFileNumber = 0;

void runTest(std::string args, int& outCode, std::vector<std::string>& outOutput, std::vector<std::string>& outStdout){
  outOutput.clear();
  outStdout.clear();
  
  std::string outputFilename(TESTS_DIR + std::to_string(currentFileNumber));
  std::string outputStdoutFilename(outputFilename + "_stdout");
  std::stringstream st;
  st << BIN_NAME << " " << args << " -o " << outputFilename << " > " << outputStdoutFilename;
  std::cout << "Running: " << st.str() << std::endl;
  outCode = std::system(st.str().c_str());
  ++currentFileNumber;
  
  std::ifstream fileOutput(outputFilename);
  std::ifstream fileStdout(outputStdoutFilename);
  
  std::string line;
  if (fileOutput.is_open())
  {
    while (std::getline(fileOutput, line)) {
      outOutput.push_back(line);
    }
  }

  if (fileStdout.is_open())
  {
    while (std::getline(fileStdout, line)) {
      outStdout.push_back(line);
    }
  }
}

TEST_CASE("Testing sift itself like a blackbox", "[system]") {
  int code = -1;
  std::vector<std::string> outputVector;
  std::vector<std::string> stdoutVector;
  
  SECTION("All green quiet test") {
    runTest("-q -r samples/tests/rules/empty.json -p samples/src/define.hpp", code, outputVector, stdoutVector);
    REQUIRE(code == 0);
    REQUIRE_OUTPUT_SIZE(stdoutVector, 0);
    REQUIRE_OUTPUT_SIZE(outputVector, 2);
  }
  
  SECTION("Run sift on itself with no errors") {
    runTest("-q -r samples/tests/rules/empty.json -p samples/SIFT", code, outputVector, stdoutVector);
    REQUIRE(code == 0);
    REQUIRE_OUTPUT_SIZE(stdoutVector, 0);
    REQUIRE_OUTPUT_SIZE(outputVector, 2);
  }

}
