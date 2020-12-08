include_guard(DIRECTORY)

cmake_minimum_required(VERSION 3.12)
project(TestExamples)

#add our c++ standard and optimizations
set(CMAKE_CONFIGURATION_TYPES "Release;Debug")
#Set the compiler to use c++17, depending on which compiler we use.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    #remove extra configurations, make release the default
    add_compile_options(/std:c++17)
else()
    add_compile_options(-std=c++17)
endif()

#You should normally be using debug build
#but if you do that with these tests, they can take a while to run...
if(CMAKE_CONFIGURATION_TYPES STREQUAL "Debug")
    add_definitions(-DDEBUG=1)
    add_definitions(-DJUCE_DEBUG=1)
    add_definitions(-DNDEBUG=0)
elseif(CMAKE_CONFIGURATION_TYPES STREQUAL "Release")
    add_definitions(-DDEBUG=0)
    add_definitions(-DJUCE_DEBUG=0)
    add_definitions(-DNDEBUG=1)
    #I use Ofast instead of O3 because I'd like to make sure the algorithms retains accuracy
    #Ofast can enable optimizations that reduce math accuracy be reordering operations
    #Clang masquerades as a lot of compilers, so check it first.
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options( -Ofast -fopenmp)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        add_compile_options(-Ofast)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        add_compile_options(-Ofast)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        #No fast math in msvc because it fails
        add_compile_options(/O2 /Ot /Ob2 /Oi /GL)
    #Lots of compilers can masquerade as GNU, so check for it last
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Ofast -fopenmp)
    endif()
endif()

#Make sure the user defined a path for JUCE- error if they did not.
if(NOT DEFINED JUCE_PATH)
    message(FATAL_ERROR "You must set JUCE_PATH environment variable")
endif()
# overwrite install() command with a dummy macro that is a nop
macro (install)
endmacro ()


#Find and include JUCE and its modules
add_subdirectory("${JUCE_PATH}" ./JUCE)
#Find catch2 headers in the system
find_package(Catch2 REQUIRED)

# remove install target added by JUCE, since this project should not install anything
install()# function with the given arguments
macro (install)
    _install(${ARGV})
endmacro(install)


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
        PRIVATE
        JUCE_STANDALONE_APPLICATION=1
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

#Enable test registration and include Catch for all subprojects
#We want to use CTest to run our tests because it gives us fine grained control over which tests we run
#As well as parallelizing our tests- each new test can run on an empty core
enable_testing()
include(Catch)
#Remove ZERO_CHECK target
set(CMAKE_SUPPRESS_REGENERATION true)

#If no NUM_CORES argument given, make one equal to half the cores on this system
if(NOT DEFINED NUM_CORES)
    #Set the number of parallel jobs ctest can use to the number of cores on the machine
    include(ProcessorCount)
    ProcessorCount(NUM_CORES)
    if(USE_ALL_CORES)
        math(EXPR NumCores "${NUM_CORES}/2")
    endif()
endif()

#Add the number of cores to use, as well as desired verbosity to CTest's arguments
list(APPEND CMAKE_CTEST_ARGUMENTS -j${NUM_CORES} --extra-verbose)

#Add Directories for the Tests
add_subdirectory("1. Oscillator/Examples")
add_subdirectory("1. Oscillator")
add_subdirectory("2. Filters")
