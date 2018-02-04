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

#include "scope.hpp"
#include "file.hpp"

namespace Core {
  
  Scope::Scope(ScopeType type)
    : type(type)
  {
  }
/*
  void Scope::pushChild(const Scope& child) {
    children.push_back(child);
  }

  const ScopeVector& Scope::getChildren() const {
    return children;
  }*/

  ScopeVector Scope::getDirectChildrenOfType(ScopeType type) const {
    ScopeVector children;

    for(const auto& child : children) {
      if(static_cast<unsigned int>(child.type & type) != 0) {
        children.push_back(child);
      }
    }

    return children;
  }
  
  ScopeVector Scope::getAllChildrenOfType(ScopeType type) const {
    ScopeVector stack, current, toReturn;
    stack = children;
    while(!stack.empty())
    {
      current = stack;
      stack.clear();
      for(const auto& child : current)
      {
        if(static_cast<unsigned int>(child.type & type) != 0) {
          toReturn.push_back(child);
        }
        
        stack.insert(stack.begin(), child.children.begin(), child.children.end());
      }
    }
    return toReturn;
  }

  unsigned int Scope::getDepth() const {
    unsigned int depth = 0;
    return depth;
  }
  
  std::vector<std::string> Core::Scope::getScopeLines() const
  {
    std::vector<std::string> toReturn;
    int currentLineNo = 1;
    int currentCharNo = 1;
    for(const auto& line : file->lines)
    {
      if(currentLineNo == lineNumberStart)
      {
        bool multiLine = characterNumberEnd > characterNumberStart + line.size();
        toReturn.push_back(line.substr(characterNumberStart-currentCharNo, (multiLine ? line.size() : characterNumberEnd-currentCharNo)));
        if(!multiLine)
          break;
      }
      else if(currentCharNo > characterNumberStart && currentCharNo + line.size()-1 <= characterNumberEnd) // Line after a '#define \'   
      {
        toReturn.push_back(line);
      }
      else if(currentCharNo < characterNumberEnd && currentCharNo + line.size()-1 > characterNumberEnd) // Line finishing a multiLine define
      {
        toReturn.push_back(line.substr(0, characterNumberEnd-currentCharNo));
      }
      
      currentCharNo+=line.size()+1;
      currentLineNo++;
    }
    
    return toReturn;
  }
}
