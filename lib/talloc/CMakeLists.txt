cmake_minimum_required(VERSION 2.8)
project(talloc)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=gnu99 -pedantic")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g")

set(TALLOC_DEBUG_CALLBACK true)

include_directories("../replace/")
include_directories("./bin/default/")
add_library(
	talloc SHARED
	talloc.c
)

include_directories("./utils")
add_subdirectory(test)