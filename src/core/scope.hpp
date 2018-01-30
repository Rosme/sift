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

namespace Core {

  struct File;
  
  enum class ScopeType {
    Source = 1u << 0,
    Namespace = 1u << 1,
    Class = 1u << 2,
    Enum = 1u << 3,
    FreeFunction = 1u << 4,
    ClassFunction = 1u << 5,
    Function = FreeFunction | ClassFunction,
    Conditionnal = 1u << 6,
    ClassVariable = 1u << 7,
    FunctionVariable = 1u << 8,
    GlobalVariable = 1u << 9,
    Global = GlobalVariable | FreeFunction,
    Variable = ClassVariable | FunctionVariable | GlobalVariable,
    Unknown = 1u << 10
  };

  class Scope;
  using ScopePtr = std::unique_ptr<Scope>;
  using ScopeVector = std::vector<Scope>;
  using ScopePtrVector = std::vector<ScopePtr>;

  class Scope {
  public:
    explicit Scope(ScopeType type);
    
    void pushChild(const Scope& child);
    const ScopeVector& getChildren() const;
    ScopeVector getChildrenOfType(ScopeType type) const;

  private:
    const ScopeType m_type;
    ScopeVector m_children;
    unsigned int m_lineNumber;
    File* m_file;
  };

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
    case ScopeType::Conditionnal:
      return "Conditionnal";
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
    default:
      return "Unknown";
    }
  }

  inline ScopeType ScopeType_to_enum_class(const std::string& type) {
    if(_stricmp(type.c_str(), "Source") == 0) {
      return ScopeType::Source;
    }
    
    if(_stricmp(type.c_str(), "Namespace") == 0) {
      return ScopeType::Namespace;
    }

    if(_stricmp(type.c_str(), "Class") == 0) {
      return ScopeType::Class;
    }

    if(_stricmp(type.c_str(), "Enum") == 0) {
      return ScopeType::Enum;
    }

    if(_stricmp(type.c_str(), "FreeFunction") == 0) {
      return ScopeType::FreeFunction;
    }

    if(_stricmp(type.c_str(), "ClassFunction") == 0) {
      return ScopeType::ClassFunction;
    }

    if(_stricmp(type.c_str(), "Function") == 0) {
      return ScopeType::Function;
    }

    if(_stricmp(type.c_str(), "Conditionnal") == 0) {
      return ScopeType::Conditionnal;
    }

    if(_stricmp(type.c_str(), "ClassVariable") == 0) {
      return ScopeType::ClassVariable;
    }

    if(_stricmp(type.c_str(), "FunctionVariable") == 0) {
      return ScopeType::FunctionVariable;
    }

    if(_stricmp(type.c_str(), "GlobalVariable") == 0) {
      return ScopeType::GlobalVariable;
    }

    if(_stricmp(type.c_str(), "Global") == 0) {
      return ScopeType::Global;
    }

    if(_stricmp(type.c_str(), "Variable") == 0) {
      return ScopeType::Variable;
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

}
