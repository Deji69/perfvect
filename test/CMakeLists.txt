cmake_minimum_required(VERSION 3.8.0)

##
## Options
##
option(Perfvect_Sanitizer "Build test suite with Clang sanitizer" OFF)

##
## Packages
##
find_package(Catch2 CONFIG REQUIRED)
include(Catch)

##
## Build
##
set(perfvect_tests_src
	"src/helper.h"
	"src/main.cpp"
	"src/vector_test.cpp"
	"src/static_vector_test.cpp"
	"src/small_vector_test.cpp")
add_executable(perfvect_tests ${perfvect_tests_src})
target_link_libraries(perfvect_tests Catch2::Catch2 ${PERFVECT_TARGET_NAME})

if(Perfvect_Sanitizer AND NOT MSVC)
	message(STATUS "Building test suite with Clang sanitizer")
	set(PLATFORM_LINK_FLAGS "${PLATFORM_LINK_FLAGS} -fsanitize=undefined")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PLATFORM_LINK_FLAGS}")
	target_compile_options(perfvect_tests -fsanitize=undedfined)
endif()

if(MSVC)
	target_compile_options(perfvect_tests PRIVATE /W4 /WX)
else()
	target_compile_options(perfvect_tests PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

catch_discover_tests(perfvect_tests)