include_guard(DIRECTORY)

cmake_minimum_required(VERSION 3.12)
project(TestExamples)

#add our c++ standard and optimizations
#You should normally be using debug build
#but if you don't, in this case the tests can take a while to run...
#I use Ofast instead of O3 because I'd like to make sure the algorithms retains accuracy
#Ofast can enable optimizations that reduce math accuracy be reordering operations
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options( -Ofast -fopenmp)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_options(-Ofast)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    add_compile_options(-Ofast)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/O2 /fp:fast)
#Lots of compilers can masquerade as GNU, so check for it last
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options( -Ofast -fopenmp)
endif()

#Set the compiler to use c++17, depending on which compiler we use.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/std:c++17)
else()
    add_compile_options(-std=c++17)
endif()

#Make sure the user defined a path for JUCE- error if they did not.
if(NOT DEFINED JUCE_PATH)
    message(FATAL_ERROR "You must set JUCE_PATH environment variable")
endif()
#Find and include JUCE and its modules
add_subdirectory("${JUCE_PATH}" ./JUCE)
#Find catch2 headers in the system
find_package(Catch2 REQUIRED)

#===================Common Library Code====================================
#Declare a library called common code
#We'll link in our JUCE dependencies and Catch into this, and then link this to all of our test targets
add_library(CommonCode STATIC)

#Link in Catch, juce_core and and juce_dsp. Also add in JUCE's usual flags
target_link_libraries(CommonCode
        PRIVATE
        Catch2::Catch2
        juce::juce_core
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        )

target_compile_definitions(CommonCode
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        INTERFACE
        $<TARGET_PROPERTY:CommonCode,COMPILE_DEFINITIONS>)

target_include_directories(CommonCode
        INTERFACE
        $<TARGET_PROPERTY:CommonCode,INCLUDE_DIRECTORIES>)

set_target_properties(CommonCode PROPERTIES
        POSITION_INDEPENDENT_CODE TRUE
        VISIBILITY_INLINES_HIDDEN TRUE
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)

#==========================================================================

#Add CTest to enable test registration in the subproject
include(CTest)

#Add Directories for the Tests
add_subdirectory("1. Oscillator/Examples")
add_subdirectory("1. Oscillator")
add_subdirectory("2. Filters")

#Set the number of parallel jobs ctest can use to the number of cores on the machine
include(ProcessorCount)
ProcessorCount(NumCores)
set(CTEST_PARALLEL_LEVEL NumCores)