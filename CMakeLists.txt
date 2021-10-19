cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
cmake_policy(VERSION 3.2)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------------------------------------
# Options
#--------------------------------------------------------------------
option(BUILD_TESTS "Build library unit-tests" OFF)

#--------------------------------------------------------------------
# Version control
#--------------------------------------------------------------------
file(READ ".version" VERSION_FILE)

string(REGEX MATCH "VERSION_MAJOR[ ]+([0-9]+)" _ ${VERSION_FILE})
set(VERSION_MAJOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "VERSION_MINOR[ ]+([0-9]+)" _ ${VERSION_FILE})
set(VERSION_MINOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "VERSION_PATCH[ ]+([0-9]+)" _ ${VERSION_FILE})
set(VERSION_PATCH "${CMAKE_MATCH_1}")

set(LIB_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
message(STATUS "XMLHttpRequest: Version " ${LIB_VERSION})

#--------------------------------------------------------------------
# Add sources
#--------------------------------------------------------------------
project(XMLHttpRequest VERSION ${LIB_VERSION} LANGUAGES CXX)

file (GLOB SRC_FILES source/*)
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

#--------------------------------------------------------------------
# Tests
#--------------------------------------------------------------------
if(BUILD_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()
