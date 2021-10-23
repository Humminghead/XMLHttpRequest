include(CTest)
project(XMLHttpRequestTests)

include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${LIB_INCLUDE_PATH})
link_libraries(XMLHttpRequest)

add_executable(get_request_test get_request_test.cpp)
add_executable(post_request_test post_request_test.cpp)

add_test(NAME get_request_unit_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/get_request_test)
set_tests_properties(get_request_unit_test PROPERTIES WILL_FAIL FALSE)

add_test(NAME post_request_unit_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/post_request_test)
set_tests_properties(post_request_unit_test PROPERTIES WILL_FAIL FALSE)

#https://developer.mantidproject.org/RunningTheUnitTests.html
