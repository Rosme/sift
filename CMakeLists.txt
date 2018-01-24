cmake_minimum_required(VERSION 3.0)

project(PFE)

set(SRC_MISC
    src/main.cpp
    )

set(SRC_CORE
    src/core/utils.hpp
    src/core/scope.hpp
    src/core/scope.cpp
    src/core/file.hpp
    )

SET(SRC_SYNTAX
    src/syntax/syntax_analyser.hpp
    src/syntax/syntax_analyser.cpp
    src/syntax/rule.hpp
    src/syntax/rule.cpp
    )

SET(SRC_FLOW
    src/flow/flow_analyser.hpp
    src/flow/flow_analyser.cpp
    )

SET(SRC
    ${SRC_MISC}
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
endif()

include_directories(extlibs)

add_subdirectory(tests)

add_executable(PFE ${SRC})
