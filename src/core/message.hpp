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

#include <string>
#include <rosme/smartenum.hpp>

namespace Core {

  smart_enum_class(MessageType,
                   Warning,
                   Error,
                   Unknown);

  struct Message {
    Message(MessageType type = MessageType::Unknown, const std::string& content = "", const int line = 0, const int character = 0)
      : type(type)
      , content(content)
      , line(line)
      , character(character) {}
    MessageType type;
    std::string content;
    int line;
    int character;
  };

  inline std::ostream& operator<<(std::ostream& out, const Message& message) {
//     out << to_string(message.type) << "(L" << message.line << "/C" << message.character << ")"  << " : " << message.content;
    std::string characterString(message.character != 0 ? ", Character: " + std::to_string(message.character) : "");
    out << "L" << message.line+1 << characterString << ": " << message.content;
    return out;
  }

}
