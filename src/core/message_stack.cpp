#include "message_stack.hpp"
/* MIT License
*
* Copyright (c) 2018 Jean-Sebastien Fauteux, Michel Rioux, Rapha�l Massabot
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

#include "message_stack.hpp"
#include "assert.hpp"

namespace Core {

  void MessageStack::pushMessage(const Message & message) {
    m_messages.push_back(message);
  }

  bool MessageStack::hasMessages() const {
    return m_messages.size() != 0;
  }

  const std::vector<Message>& MessageStack::getMessages() const {
    return m_messages;
  }

  std::size_t MessageStack::size() const {
    return m_messages.size();
  }

  Message MessageStack::popMessage() {
    PFE_ASSERT(hasMessages(), "Empty messages stack")
    Message message = m_messages[0];
    m_messages.erase(m_messages.begin());
    return message;
  }

}