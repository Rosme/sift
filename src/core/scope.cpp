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

  ScopeVector Scope::getDirectChildrenOfType(ScopeType type) const {
    ScopeVector directChildren;

    for(const auto& child : children) {
      if(static_cast<unsigned int>(child.type & type) != 0) {
        directChildren.push_back(child);
      }
    }

    return directChildren;
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
    
    auto currentParent = parent;
    while(currentParent) {
      ++depth;
      currentParent = currentParent->parent;
    }

    return depth;
  }

  std::string Scope::getTree() const {
    std::ostringstream tree;

    for(unsigned int i = 0; i < getDepth(); ++i) {
      tree << "  ";
    }
    tree << to_string(type) << " : " << name;
    for(const auto& child : children) {
      tree << "\n" << child.getTree();
    }

    return tree.str();
  }

  bool Scope::isWithinOtherScope(const Scope& other) const {
    //Start line is before other or end line is after other
    if(lineNumberStart < other.lineNumberStart || lineNumberEnd > other.lineNumberEnd) {
      return false;
    }

    //Same starting line but start character is before other
    if(lineNumberStart == other.lineNumberStart && characterNumberStart < other.characterNumberStart) {
      return false;
    }

    //Same ending line but end character is after other
    if(lineNumberEnd == other.lineNumberEnd && characterNumberEnd > other.characterNumberEnd) {
      return false;
    }

    return true;
  }

  bool Scope::isOfType(ScopeType typeToVerify) const {
    return static_cast<unsigned int>(type & typeToVerify) != 0;
  }

  ScopeVector::iterator Scope::begin() {
    return children.begin();
  }

  ScopeVector::const_iterator Scope::begin() const {
    return children.begin();
  }

  ScopeVector::iterator Scope::end() {
    return children.end();
  }

  ScopeVector::const_iterator Scope::end() const {
    return children.end();
  }
  
  std::vector<std::string> Core::Scope::getScopeLines() const
  {
    std::vector<std::string> toReturn;
    unsigned int currentLineNo = 1;
    unsigned int currentCharNo = 1;
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
