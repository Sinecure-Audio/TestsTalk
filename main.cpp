//Tell catch that we want it to generate a main function that runs our tests.
// We need to define this before we include our catch header
#define CATCH_CONFIG_MAIN

// Include catch in the main translation unit so that the generated main function is actually included in the output binary
#include <catch2/catch.hpp>