# MIT License
# 
# Copyright (c) 2018 Jean-Sebastien Fauteux, Michel Rioux, Raphaël Massabot
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

project("tests")

SET(TESTS_INC
  catch.hh
  )

set(TESTS_TESTS
  main.cpp
  tests_json.cpp
  tests_message.cpp
  tests_scope.cpp
  tests_rule.cpp
  tests_samples.cpp
  tests_flow.cpp
  )

if(RUN_SYSTEM_TESTS)
  message("Will run system tests")
  list(APPEND TESTS_TESTS tests_system.cpp)
endif(RUN_SYSTEM_TESTS)

include_directories(../extlib SYSTEM)
include_directories(../src)

source_group("SIFT_Source" FILES ${SRC})
source_group("Tests" FILES ${TESTS_TESTS})

add_executable("tests" ${TESTS_INC} ${TESTS_TESTS}
  "${CMAKE_SOURCE_DIR}/src/sift.cpp" 
  ${SRC}
)

add_custom_command(TARGET tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:tests>/temp-system-tests"
)
add_custom_command(TARGET tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:tests>/temp-system-tests"
)

add_custom_command(TARGET tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:sift>" "$<TARGET_FILE_DIR:tests>"
)
add_custom_command(TARGET tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/samples $<TARGET_FILE_DIR:tests>/samples
)
add_custom_command(TARGET tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src $<TARGET_FILE_DIR:tests>/samples/SIFT
)
