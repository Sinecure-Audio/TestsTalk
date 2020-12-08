![image](https://github.com/Sinecure-Audio/TestsTalk/workflows/Run_Tests/badge.svg)

This is a collection of test examples made for my talk, "Lessons Learned From a Year of Unit Testing Audio Code" that I presented as part of ADC'20. They are meant to demonstrate the application of TDD to DSP development, and the verification of correct behavior of simple musical DSP processors.

All of the examples require Catch2 and CMake to be installed, as well as a copy of JUCE. This project requires CMake 3.12 or higher. You can find instructions for installing Catch2 here: 
https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md#installing-catch2-from-git-repository 

The tests also use my units library as a submodule, which requires you to run `git submodule update --init --recursive` from inside the repository after you pull the repo. 

You can build the tests by making a build directory and calling `cmake "THIS_DIRECTORY" -D"JUCE_PATH"`, where `THIS_DIRECTORY` is the directory of the repo, and `JUCE_PATH` is the path to your JUCE folder. It is a good idea to make sure the build directory is not in one of the source code directories of this project.

After building the tests by running ALL_BUILD in your IDE, or using `make all` or `ninja all` if you use command line tools, (You can specify different build systems with the `-G` flag in cmake), you can then run the tests by calling `ctest` from your build directory. Passing `-j` followed by a number will use that many cores to run the tests in parallel. Ninja and make have test targets that allow you to see the results of the tests in real time. These can be run by using `make tests` or `ninja tests`. Xcode and Visual Studio have RUN_TESTS targets that run the tests when built. Visual Studio will output the results to its console. Xcode writes the result of the tests to `XCODE_PROJECT/Testing/Temporary/LastTest.log`, where `XCODE_PROJECT` is the location of your Xcode project file. 

If you don't intend to run ctest from the command line, you can set the number of cores/threads to use by passing `-D"NUM_CORES"` to cmake, where `NUM_CORES` is the number of cores/threads you want to use. The default is half of the cores cmake determines are on your computer.