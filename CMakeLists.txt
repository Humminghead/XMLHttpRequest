cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
cmake_policy(VERSION 3.2)

include(GNUInstallDirs)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

project(XMLHttpRequest VERSION ${LIB_VERSION} LANGUAGES CXX)
#--------------------------------------------------------------------
# Update module path
#--------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

#--------------------------------------------------------------------
# Add third party software
#--------------------------------------------------------------------
add_subdirectory(${PROJECT_SOURCE_DIR}/third-party)

#--------------------------------------------------------------------
# Add sources
#--------------------------------------------------------------------

find_package(Boost 1.65 REQUIRED COMPONENTS regex thread)
find_package(nghttp2 REQUIRED)

file (GLOB SRC_FILES source/*)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES} $<TARGET_OBJECTS:url-parser>)
#add_dependencies(${PROJECT_NAME} url-parser)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE
     -lboost_regex
     -lboost_thread
     -lcrypto
     -lssl
     -pthread
     ${NGHTTP2_LIBRARIES}
     ) 

#--------------------------------------------------------------------
# Create install target
#--------------------------------------------------------------------
set(LIB_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE HEADER_FILES ${LIB_INCLUDE_PATH}/*.h)

foreach(header_path ${HEADER_FILES})
    string(CONCAT INSTALLED_INCLUDES "${INSTALLED_INCLUDES}${header_path}\;")
endforeach()

set_target_properties(${PROJECT_NAME} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_LIBDIR}
      PUBLIC_HEADER ${INSTALLED_INCLUDES}
)

install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

#--------------------------------------------------------------------
# Tests
#--------------------------------------------------------------------
option(BUILD_TESTING "Enables unit tests" OFF)
if(BUILD_TESTING)
    message( STATUS "\"BUILD_TESTING\" option is ON")

    enable_testing()
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

#--------------------------------------------------------------------
# Options
#--------------------------------------------------------------------
option(ENABLE_DEBUG_SSL "Enables decode of tls encrypted traffic in wireshark (/tmp/ssl-key-log.txt)" OFF)
if(ENABLE_DEBUG_SSL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_SSL)
endif()
