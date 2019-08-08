cmake_minimum_required(VERSION 3.8)
project(perfvect VERSION 0.1.0)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	message(" [INFO] VCPKG CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

if(MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(HEADERS ${HEADERS}
	${PROJECT_SOURCE_DIR}/include/perfvect/static_vector.h
	${PROJECT_SOURCE_DIR}/include/perfvect/small_vector.h
	${PROJECT_SOURCE_DIR}/include/perfvect/auto_vector.h
)
add_library(perfvect INTERFACE)
add_library(perfvect::perfvect ALIAS perfvect)
target_include_directories(perfvect INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_compile_features(perfvect INTERFACE cxx_std_17)

enable_testing()
add_subdirectory(test)