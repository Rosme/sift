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

#include "core/message_stack.hpp"

Core::MessageStack createMessageStack(int size) {
  Core::MessageStack stack;
  for(int i = 0; i < size; ++i) {
    if(i % 2 == 0) {
      stack.pushMessage({Core::MessageType::Warning, std::to_string(i)});
    } else {
      stack.pushMessage({Core::MessageType::Error, std::to_string(i)});
    }
  }
  return stack;
}

TEST_CASE("Testing Message Stack", "[message]") {

  SECTION("Pushing on the stack") {
    auto stack = createMessageStack(10);

    REQUIRE(stack.size() == 10);
  }

  SECTION("Popping 5 of the stack") {
    auto stack = createMessageStack(10);
    REQUIRE(stack.size() == 10);

    REQUIRE(stack.popMessage().type == Core::MessageType::Warning);
    REQUIRE(stack.popMessage().type == Core::MessageType::Error);
    REQUIRE(stack.popMessage().type == Core::MessageType::Warning);
    REQUIRE(stack.popMessage().type == Core::MessageType::Error);
    REQUIRE(stack.popMessage().type == Core::MessageType::Warning);


    REQUIRE(stack.size() == 5);
  }

  SECTION("Pushing 5 after pop") {
    auto stack = createMessageStack(10);
    for(int i = 0; i < 5; ++i) {
      stack.popMessage();
    }

    REQUIRE(stack.size() == 5);

    for(int i = 0; i < 5; ++i) {
      if(i % 2 == 0) {
        stack.pushMessage({Core::MessageType::Warning, std::to_string(i)});
      } else {
        stack.pushMessage({Core::MessageType::Error, std::to_string(i)});
      }
    }

    REQUIRE(stack.size() == 10);
  }

}