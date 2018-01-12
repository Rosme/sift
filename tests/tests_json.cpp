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

#include "core/utils/json.hpp"

TEST_CASE("Testing JSON utils", "[json]") {
  
  nlohmann::json json;
  const std::string fileName = "tests.json";
  const std::string jsonKey = "TestValue";
  const unsigned int jsonValue = 5;
  
  SECTION("Writing to json file") {
    
    json[jsonKey] = jsonValue;
    
    Core::write_json_file(fileName, json);
    
    {
      std::ifstream file(fileName);
      REQUIRE(file.is_open());
    }
  }
  
  SECTION("Reading from json file") {
    json = Core::read_json_file(fileName);
    
    REQUIRE(json.size() == 1);
  }
  
  SECTION("Verifying Value in json") {
    json = Core::read_json_file(fileName);
    
    REQUIRE(json[jsonKey].is_number());
    REQUIRE(json[jsonKey] == jsonValue);
  }
  
}
