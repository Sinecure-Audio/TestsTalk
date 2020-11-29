cmake_minimum_required(VERSION 3.12)
project(OscillatorExamples)

#Every target needs to have this defined,
#By using add_compile_definitions, we add the following to all targets defined in this file
add_compile_definitions(JUCE_STANDALONE_APPLICATION=1)

#Make a target for the oscillator examples from the talk
#Then add all of the source files for it
#Because the tests all have the same name, you can only build a single file at once
add_executable(OscillatorExamples
        "${CMAKE_CURRENT_LIST_DIR}/../../TestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/1. A Simple Test/OscillatorTests.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/2. A Better Perform Test/OscillatorTests.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/3. Changing The Sampling Rate/OscillatorTests.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/4. Changing The Frequency/OscillatorTests.cpp"
        )

#Link our common libraries to the Oscillator Exampels target
target_link_libraries(OscillatorExamples PRIVATE CommonCode)

#If we don't use catch_discover_tests, CTest will only parallelize at the binary level
#Meaning, each test would need to be in its own binary for maximum parallelization
catch_discover_tests(OscillatorExamples)