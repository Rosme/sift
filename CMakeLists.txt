# MIT License
#
# Copyright (c) 2018 Jean-Sebastien Fauteux, Michel Rioux, Rapha�l Massabot
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.0)

project(sift)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)
set(VERSION_PATCH 0)

option(USE_SIFT_ASSERT "Use SIFT defined assert" ON)
option(GENERATE_JAVASCRIPT "Generate/populate the .js with rules" OFF)
option(RUN_UNIT_TESTS "Run the unit tests" ON)
option(RUN_SYSTEM_TESTS "Run the (lengthy) system tests" OFF)

# Default to debug for now
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(VERSION "v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${CMAKE_BUILD_TYPE}")
message("Building ${PROJECT_NAME} ${VERSION}")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(DEBUG_BUILD 1)
else()
  set(RELEASE_BUILD 1)
endif()

configure_file("config.hpp.in" "${CMAKE_SOURCE_DIR}/src/core/config.hpp" @ONLY)

set(SRC_MAIN
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/sift.hpp
    ${CMAKE_SOURCE_DIR}/src/sift.cpp
    )

set(SRC_CORE
    ${CMAKE_SOURCE_DIR}/src/core/utils.hpp
    ${CMAKE_SOURCE_DIR}/src/core/scope.hpp
    ${CMAKE_SOURCE_DIR}/src/core/scope.cpp
    ${CMAKE_SOURCE_DIR}/src/core/file.hpp
    ${CMAKE_SOURCE_DIR}/src/core/assert.hpp
    ${CMAKE_SOURCE_DIR}/src/core/message.hpp
    ${CMAKE_SOURCE_DIR}/src/core/message_stack.hpp
    ${CMAKE_SOURCE_DIR}/src/core/message_stack.cpp
    ${CMAKE_SOURCE_DIR}/src/core/config.hpp
    ${CMAKE_SOURCE_DIR}/src/core/constants.hpp
    ${CMAKE_SOURCE_DIR}/src/core/scope_extractor.hpp
    ${CMAKE_SOURCE_DIR}/src/core/cpp_scope_extractor.hpp
    ${CMAKE_SOURCE_DIR}/src/core/cpp_scope_extractor.cpp
    )

SET(SRC_SYNTAX
    ${CMAKE_SOURCE_DIR}/src/syntax/syntax_analyser.hpp
    ${CMAKE_SOURCE_DIR}/src/syntax/syntax_analyser.cpp
    ${CMAKE_SOURCE_DIR}/src/syntax/cpp_syntax_analyser.hpp
    ${CMAKE_SOURCE_DIR}/src/syntax/cpp_syntax_analyser.cpp
    ${CMAKE_SOURCE_DIR}/src/syntax/rule.hpp
    ${CMAKE_SOURCE_DIR}/src/syntax/rule.cpp
    )

SET(SRC_FLOW
    ${CMAKE_SOURCE_DIR}/src/flow/flow_analyser.hpp
    ${CMAKE_SOURCE_DIR}/src/flow/flow_analyser.cpp
    ${CMAKE_SOURCE_DIR}/src/flow/cpp_flow_analyser.hpp
    ${CMAKE_SOURCE_DIR}/src/flow/cpp_flow_analyser.cpp
    )

SET(SRC
    ${SRC_CORE}
    ${SRC_SYNTAX}
    ${SRC_FLOW}
    )

source_group("core" FILES ${SRC_CORE})
source_group("syntax" FILES ${SRC_SYNTAX})
source_group("flow" FILES ${SRC_FLOW})


if(UNIX)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
  check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
  check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
  check_cxx_compiler_flag("-std=c++0X" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX1Y)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
  elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_THREAD_SAFE")

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

include_directories(extlibs SYSTEM)

if(USE_SIFT_ASSERT)
    add_definitions(-DUSE_SIFT_ASSERT)
endif(USE_SIFT_ASSERT)

if(GENERATE_JAVASCRIPT)
  execute_process(COMMAND python generate_rules.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/configgen/)
endif(GENERATE_JAVASCRIPT)

add_executable(${PROJECT_NAME} ${SRC_MAIN} ${SRC})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/samples $<TARGET_FILE_DIR:${PROJECT_NAME}>/samples
                    )
file(COPY ${CMAKE_SOURCE_DIR}/samples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(RUN_UNIT_TESTS)
  add_subdirectory(tests)
endif(RUN_UNIT_TESTS)
