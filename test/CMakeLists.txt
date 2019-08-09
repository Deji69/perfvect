cmake_minimum_required(VERSION 3.8.0)

##
## Options
##
option(Perfvect_Sanitizer "Build test suite with Clang sanitizer" OFF)

if(Perfvect_Sanitizer)
	message(STATUS "Building test suite with Clang sanitizer")
	set(PLATFORM_LINK_FLAGS "${PLATFORM_LINK_FLAGS} -fsanitize=undefined")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PLATFORM_LINK_FLAGS}")
	add_compile_options(-fsanitize=undedfined)
endif()

if(MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

##
## Packages
##
find_package(Catch2 CONFIG REQUIRED)
include(Catch)

##
## Build
##
set(perfvect_tests_src "src/main.cpp" "src/static_vector_test.cpp")
add_executable(perfvect_tests ${perfvect_tests_src})
target_link_libraries(perfvect_tests Catch2::Catch2 ${PERFVECT_TARGET_NAME})
catch_discover_tests(perfvect_tests)