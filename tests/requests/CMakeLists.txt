include(CTest)

project(XMLHttpRequestTests)

add_executable(test0 requests_test.cpp)

enable_testing()
add_test(NAME requests_unit_test COMMAND requests_test)
set_tests_properties(requests_unit_test PROPERTIES WILL_FAIL TRUE)

#https://developer.mantidproject.org/RunningTheUnitTests.html
