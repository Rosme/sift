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

#include <functional>
#include <vector>
#include <memory>
#include <string>

#include "file.hpp"
#include "utils.hpp"

namespace Core {
  
  enum class ScopeType {
    Source = 1u << 0, /* Root scope of a file */
    Namespace = 1u << 1,
    Class = 1u << 2,
    Enum = 1u << 3,
    FreeFunction = 1u << 4,
    ClassFunction = 1u << 5,
    Conditional = 1u << 6,
    ClassVariable = 1u << 7,
    FunctionVariable = 1u << 8,
    GlobalVariable = 1u << 9,
    GlobalDefine = 1u << 10,
    SingleLineComment = 1u << 11,
    MultiLineComment = 1u << 12,
    StringLiteral = 1u << 13,
    Unknown = 1u << 20,
    All = 0xffff,
    Global = GlobalVariable | GlobalDefine | FreeFunction,
    Variable = ClassVariable | FunctionVariable | GlobalVariable,
    Comment = SingleLineComment | MultiLineComment,
    Function = FreeFunction | ClassFunction
  };

  struct Scope;
  using ScopeVector = std::vector<Scope>;

  struct Scope {
    Scope() {};
    Scope(ScopeType type);
    
    ScopeVector getDirectChildrenOfType(ScopeType type) const;
    ScopeVector getAllChildrenOfType(ScopeType type) const;
    unsigned int getDepth() const;
    std::string getTree() const; //Debug Function
    bool isWithinOtherScope(const Scope& other) const;
    bool isLineWithinScope(unsigned int line) const;
    bool isOfType(ScopeType type) const;
    bool isMultiline() const;
    ScopeVector::iterator begin();
    ScopeVector::const_iterator begin() const;
    ScopeVector::iterator end();
    ScopeVector::const_iterator end() const;

    ScopeType type;
    ScopeVector children;
    unsigned int lineNumberStart = 0;
    unsigned int lineNumberEnd = 0;
    unsigned int characterNumberStart = 0;
    unsigned int characterNumberEnd = 0;
    std::string name;
    std::vector<std::string> getScopeLines() const;
    File* file = nullptr;
    Scope* parent = nullptr;
  };

  inline bool operator==(const Scope& lhs, const Scope& rhs) {
    bool isSameNumbers = lhs.lineNumberStart == rhs.lineNumberStart
      && lhs.lineNumberEnd == rhs.lineNumberEnd
      && lhs.characterNumberStart == rhs.characterNumberStart
      && lhs.characterNumberEnd == rhs.characterNumberEnd;

    bool sameFile = true;
    if(lhs.file && rhs.file) {
      sameFile = lhs.file->filename == rhs.file->filename;
    } else if((lhs.file && !rhs.file) || (!lhs.file && rhs.file)) {
      sameFile = false;
    } else {
      sameFile = true;
    }

    return isSameNumbers && sameFile;
  }

  inline bool operator!=(const Scope& lhs, const Scope& rhs) {
    return !(lhs == rhs);
  }

  std::string to_string(ScopeType type);
  inline std::ostream& operator<<(std::ostream& out, const Scope& scope) {
    if(!scope.file) {
      return out;
    }

    out << "Scope Name: " << scope.name << "\n"
      << "Type: " << to_string(scope.type) << "\n"
      << "Depth:" << scope.getDepth() << "\n"
      << "Start Line" << scope.lineNumberStart << "\n"
      << "End Line" << scope.lineNumberEnd << "\n"
      << "Character Start" << scope.characterNumberStart << "\n"
      << "Character End" << scope.characterNumberEnd << "\n"
      << "Content:\n";
    if(scope.lineNumberStart == scope.lineNumberEnd) {
      const std::string& line = scope.file->lines[scope.lineNumberStart];
      out << line.substr(scope.characterNumberStart, scope.characterNumberEnd-scope.characterNumberStart+1);
    } else {
      for(unsigned int i = scope.lineNumberStart; i <= scope.lineNumberEnd; ++i) {
        out << scope.file->lines[i] << "\n";
      }
    }

    return out;
  }

  inline std::string to_string(ScopeType type) {
    switch(type) {
    case ScopeType::Source:
      return "Source";
    case ScopeType::Namespace:
      return "Namespace";
    case ScopeType::Class:
      return "Class";
    case ScopeType::Enum:
      return "Enum";
    case ScopeType::FreeFunction:
      return "FreeFunction";
    case ScopeType::ClassFunction:
      return "ClassFunction";
    case ScopeType::Function:
      return "Function";
    case ScopeType::Conditional:
      return "Conditional";
    case ScopeType::ClassVariable:
      return "ClassVariable";
    case ScopeType::FunctionVariable:
      return "FunctionVariable";
    case ScopeType::GlobalVariable:
      return "GlobalVariable";
    case ScopeType::Global:
      return "Global";
    case ScopeType::Variable:
      return "Variable";
    case ScopeType::GlobalDefine:
      return "Define";
    case ScopeType::SingleLineComment:
      return "SingleLineComment";
    case ScopeType::MultiLineComment:
      return "MultiLineComment";
    case ScopeType::Comment:
      return "Comment";
    case ScopeType::All:
      return "All";
    default:
      return "Unknown";
    }
  }

  inline ScopeType ScopeType_to_enum_class(const std::string& type) {
    if(string_case_compare(type, "Source")) {
      return ScopeType::Source;
    }
    
    if(string_case_compare(type, "Namespace")) {
      return ScopeType::Namespace;
    }

    if(string_case_compare(type, "Class")) {
      return ScopeType::Class;
    }

    if(string_case_compare(type, "Enum")) {
      return ScopeType::Enum;
    }

    if(string_case_compare(type, "FreeFunction")) {
      return ScopeType::FreeFunction;
    }

    if(string_case_compare(type, "ClassFunction")) {
      return ScopeType::ClassFunction;
    }

    if(string_case_compare(type, "Function")) {
      return ScopeType::Function;
    }

    if(string_case_compare(type, "Conditional")) {
      return ScopeType::Conditional;
    }

    if(string_case_compare(type, "ClassVariable")) {
      return ScopeType::ClassVariable;
    }

    if(string_case_compare(type, "FunctionVariable")) {
      return ScopeType::FunctionVariable;
    }

    if(string_case_compare(type, "GlobalVariable")) {
      return ScopeType::GlobalVariable;
    }

    if(string_case_compare(type, "Global")) {
      return ScopeType::Global;
    }

    if(string_case_compare(type, "Variable")) {
      return ScopeType::Variable;
    }

    if(string_case_compare(type, "SingleLineComment")) {
      return ScopeType::SingleLineComment;
    }

    if(string_case_compare(type, "MultiLineComment")) {
      return ScopeType::MultiLineComment;
    }

    if(string_case_compare(type, "Comment")) {
      return ScopeType::Comment;
    }
    
    if(string_case_compare(type, "All")) {
      return ScopeType::All;
    }

    return ScopeType::Unknown;
  }

  inline ScopeType operator&(ScopeType lhs, ScopeType rhs) {
    using Underlying = std::underlying_type<ScopeType>::type;
    return static_cast<ScopeType>(
      static_cast<Underlying>(lhs) &
      static_cast<Underlying>(rhs)
      );
  }

  inline ScopeType operator|(ScopeType lhs, ScopeType rhs) {
    using Underlying = std::underlying_type<ScopeType>::type;
    return static_cast<ScopeType>(
      static_cast<Underlying>(lhs) |
      static_cast<Underlying>(rhs)
      );
  }

  inline bool isScopeTypeOfType(ScopeType current, ScopeType toVerifyAgainst) {
    return static_cast<unsigned int>(current & toVerifyAgainst) != 0;
  }
}
