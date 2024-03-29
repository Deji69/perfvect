cmake_minimum_required(VERSION 3.8.0)

##
## Project
##
project(perfvect VERSION 0.1.0 LANGUAGES CXX)

##
## Include
##

##
## Options
##
option(Perfvect_Testing "Build unit tests" ON)
option(Perfvect_Install "Install CMake targets" ON)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	message(" [INFO] VCPKG CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
endif()

##
## Config
##
include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY          ${CMAKE_BINARY_DIR}/bin)

set(PERFVECT_VERSION                        ${PROJECT_VERSION})
set(PERFVECT_TARGET_NAME                    ${PROJECT_NAME})
set(PERFVECT_CONFIG_INSTALL_DIR             "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE INTERNAL "")
set(PERFVECT_INCLUDE_BUILD_DIR              "${PROJECT_SOURCE_DIR}/include/")
set(PERFVECT_INCLUDE_INSTALL_DIR            "${CMAKE_INSTALL_INCLUDEDIR}")
set(PERFVECT_TARGETS_EXPORT_NAME            "${PROJECT_NAME}Targets")
set(PERFVECT_CMAKE_CONFIG_TEMPLATE          "cmake/config.cmake.in")
set(PERFVECT_CMAKE_CONFIG_DIR               "${CMAKE_CURRENT_BINARY_DIR}")
set(PERFVECT_CMAKE_VERSION_CONFIG_FILE      "${PERFVECT_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(PERFVECT_CMAKE_PROJECT_CONFIG_FILE      "${PERFVECT_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")
set(PERFVECT_CMAKE_PROJECT_TARGETS_FILE     "${PERFVECT_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Targets.cmake")

message(STATUS "Version: ${PERFVECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

##
## Target
##
add_library(${PERFVECT_TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PERFVECT_TARGET_NAME} ALIAS ${PERFVECT_TARGET_NAME})
target_compile_features(perfvect INTERFACE cxx_std_17)
target_include_directories(${PERFVECT_TARGET_NAME} INTERFACE $<BUILD_INTERFACE:${PERFVECT_INCLUDE_BUILD_DIR}> $<INSTALL_INTERFACE:include>)

set(PERFVECT_ADD_NATVIS TRUE)
set(PERFVECT_NATVIS_FILE "perfvect.natvis")
target_sources(${PERFVECT_TARGET_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PERFVECT_NATVIS_FILE}> $<INSTALL_INTERFACE:${PERFVECT_NATVIS_FILE}>)

##
## Tests
##
include(CTest)
if(BUILD_TESTING AND Perfvect_Testing)
	enable_testing()
	add_subdirectory(test)
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PERFVECT_CMAKE_VERSION_CONFIG_FILE} VERSION ${PERFVECT_VERSION} COMPATIBILITY SameMajorVersion)
configure_file(${PERFVECT_CMAKE_CONFIG_TEMPLATE} ${PERFVECT_CMAKE_PROJECT_CONFIG_FILE} @ONLY)

##
## Install
##
if(Perfvect_Install)
	install(DIRECTORY ${PERFVECT_INCLUDE_BUILD_DIR} DESTINATION ${PERFVECT_INCLUDE_INSTALL_DIR})
	install(
		FILES ${PERFVECT_CMAKE_PROJECT_CONFIG_FILE} ${PERFVECT_CMAKE_VERSION_CONFIG_FILE}
		DESTINATION ${PERFVECT_CONFIG_INSTALL_DIR}
	)
	export(TARGETS ${PERFECT_TARGET_NAME} NAMESPACE ${PROJECT_NAME}:: FILE ${PERFVECT_CMAKE_PROJECT_TARGETS_FILE})
	install(TARGETS ${PERFVECT_TARGET_NAME} EXPORT ${PERFVECT_TARGETS_EXPORT_NAME} INCLUDES DESTINATION ${PERFVECT_INCLUDE_INSTALL_DIR})
	install(EXPORT ${PERFVECT_TARGETS_EXPORT_NAME} NAMESPACE ${PROJECT_NAME}:: DESTINATION ${PERFVECT_CONFIG_INSTALL_DIR})
	
	if (PERFVECT_ADD_NATVIS)
		install (FILES ${PERFVECT_NATVIS_FILE} DESTINATION .)
	endif ()
endif()