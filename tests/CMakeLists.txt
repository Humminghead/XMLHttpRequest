file(GLOB UNIT_TESTS */CMakeLists.txt )
foreach(TEST ${UNIT_TESTS})
    string(REGEX REPLACE "\/CMakeLists.txt" "" TEST_DIR ${TEST})    
    add_subdirectory(${TEST_DIR})
endforeach()

