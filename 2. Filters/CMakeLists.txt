cmake_minimum_required(VERSION 3.12)
project(FilterTests)

#Find catch2 headers in the system
#find_package(Catch2 REQUIRED)

#Every target needs to have this defined,
#By using add_compile_definitions, we add the following to all targets defined in this file
add_compile_definitions(JUCE_STANDALONE_APPLICATION=1)

#Make a target for the utilities the filter tests rely on
#Then add all of the source files for it
add_executable(FilterUtilityTests
        "${CMAKE_CURRENT_LIST_DIR}/../TestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Signal Analysis/FFT/FFTTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Signal Analysis/UtilsTest.cpp"
        )

#Link our common libraries to the Filter Utilities target
target_link_libraries(FilterUtilityTests PRIVATE CommonCode)

#Make a target for filter tests and add all of the source files for it
add_executable(FilterTests
        "${CMAKE_CURRENT_LIST_DIR}/../TestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/FilterTests.cpp"
        )

#Link our common libraries to the Filter tests target
target_link_libraries(FilterTests PRIVATE CommonCode)

#If we don't use catch_discover_tests, CTest will only parallelize at the binary level
#Meaning, each test would need to be in its own binary for maximum parallelization
catch_discover_tests(FilterUtilityTests)
catch_discover_tests(FilterTests)