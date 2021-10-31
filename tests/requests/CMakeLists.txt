include(CTest)
project(XMLHttpRequestTests)

include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${LIB_INCLUDE_PATH})
link_libraries(XMLHttpRequest)

#--------------------------------------------------------------------
# Executable
#--------------------------------------------------------------------

add_executable(host_avail_test ping.cpp)
add_executable(get_request_test get_request_test.cpp)

#--------------------------------------------------------------------
# Tests
#--------------------------------------------------------------------

add_test(NAME host_avail_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/host_avail_test mapgl.2gis.com)
add_test(NAME get_request_unit_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/get_request_test)
#add_test(NAME post_request_unit_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/post_request_test)

#--------------------------------------------------------------------
# Tests properties
#--------------------------------------------------------------------

set_tests_properties(host_avail_test PROPERTIES FIXTURES_SETUP PingHost)
set_tests_properties(host_avail_test PROPERTIES PASS_REGULAR_EXPRESSION "Passed")
set_tests_properties(get_request_unit_test PROPERTIES FIXTURES_REQUIRED RunTests)
set_tests_properties(get_request_unit_test PROPERTIES PASS_REGULAR_EXPRESSION "Status code: 200")

#set_tests_properties(post_request_unit_test PROPERTIES WILL_FAIL FALSE)
#set_tests_properties(post_request_unit_test PROPERTIES PASS_REGULAR_EXPRESSION "Status code: 200")

#https://developer.mantidproject.org/RunningTheUnitTests.html
