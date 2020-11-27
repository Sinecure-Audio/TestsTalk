cmake_minimum_required(VERSION 3.12)
project(OscillatorTests)

#Find catch2 headers in the system
find_package(Catch2 REQUIRED)

#Make a target for the utilities the oscillators tests rely on
#Then add all of the source files for it
add_executable(OscillatorUtilityTests
        ../TestMain.cpp
        "${CMAKE_CURRENT_LIST_DIR}/../Utilities/DecibelMatcherTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../Utilities/RandomTests.cpp"
        )

#Link our common libraries to the Oscillator Utilities target
target_link_libraries(OscillatorUtilityTests PRIVATE CommonCode)

#Make a target for oscillators tests and add all of the source files for it
add_executable(OscillatorTests
        ../TestMain.cpp
        "${CMAKE_CURRENT_LIST_DIR}/PhasorTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/OscillatorTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/WaveformTests.cpp"
        )

#Link our common libraries to the Oscillator tests target
target_link_libraries(OscillatorTests PRIVATE CommonCode)


#Include CTest, Catch, and allow CTest to discover all of our catch tests
#We want to use CTest to run our tests because it gives us fine grained control over which tests we run
#As well as parallelizing our tests- each new test can run on an empty core
#If we don't use catch_discover_tests, CTest will only parallelize at the binary level
#Meaning, each test would need to be in its own binary for maximum parallelization
include(CTest)
include(Catch)
catch_discover_tests(OscillatorTests OscillatorUtilityTests)

#Every target needs to have this defined,
#By using add_compile_definitions, we add the following to all targets defined in this file
add_compile_definitions(JUCE_STANDALONE_APPLICATION=1)