cmake_minimum_required(VERSION 3.12)
project(TestExamples)

#Find catch2 headers in the system
find_package(Catch2 REQUIRED)

#Find and include JUCE and its modules
add_subdirectory("${JUCE_PATH}" ./JUCE)

add_executable(TestExamples
        #Add a main file for the executable
        main.cpp

#       These are the completed slide examples from the talk.
#       The oscillator tests explore a more thourough version of each of these
#        "${CMAKE_CURRENT_LIST_DIR}/1. Oscillator/Examples/1. A Simple Test/OscillatorTests.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/1. Oscillator/Examples/2. A Better Perform Test/OscillatorTests.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/1. Oscillator/Examples/3. Changing The Sampling Rate/OscillatorTests.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/1. Oscillator/Examples/4. Changing The Frequency/OscillatorTests.cpp"

#        #Add the Tests for utilities that both the oscillator and filter tests rely on
        "${CMAKE_CURRENT_LIST_DIR}/Utilities/DecibelMatcherTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Utilities/RandomTests.cpp"
#
#        #Add the Tests for the oscillator class
        "${CMAKE_CURRENT_LIST_DIR}/1. Oscillator/OscillatorTests.cpp"
#
#        #Add the Tests for utilities that the filter tests rely on
        "${CMAKE_CURRENT_LIST_DIR}/2. Filters/Signal Analysis/FFT/FFTTests.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/2. Filters/Signal Analysis/UtilsTest.cpp"
#
#        #Add the tests for the filter
        "${CMAKE_CURRENT_LIST_DIR}/2. Filters/FilterTests.cpp"
        )

target_link_libraries(TestExamples
        PRIVATE
        Catch2::Catch2

        juce::juce_core
        juce::juce_dsp
        )

# Set the c++ standard and make sure we're not using GNU extensions
set_target_properties(TestExamples PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)

#Specify the tests as a standalone application so that JUCE doesn't complain about missing settings
target_compile_definitions(TestExamples PRIVATE JUCE_STANDALONE_APPLICATION=1)
#Add optimizations so that the filter tests don't take forever to run
target_compile_options(TestExamples PRIVATE -O3 -fopenmp)
