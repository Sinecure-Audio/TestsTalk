#include <catch2/catch.hpp>

#include "Signal Analyzers.h"

TEST_CASE ("Cumulative Average", "[Buffer Accumulator]")
{
    BufferAverager<double, 10> accumulators{};
    std::array<double, 10> buf{0.0};
    std::fill(buf.begin(), buf.end(), 1.0);

    accumulators.perform(buf);
    for(auto&& cma : accumulators.getBuffer())
        REQUIRE(cma.getAverage() == 1);

    std::fill(buf.begin(), buf.end(), 2.0);
    accumulators.perform(buf);
    for(auto&& cma : accumulators.getBuffer())
        REQUIRE(cma.getAverage() == 1.5);

    std::fill(buf.begin(), buf.end(), 3.0);
    accumulators.perform(buf);
    for(auto&& cma : accumulators.getBuffer())
        REQUIRE(cma.getAverage() == 2);
    accumulators.reset();

    for(auto&& cma : accumulators.getBuffer())
        REQUIRE(cma.getAverage() == 0);
    accumulators.perform(buf);
    for(auto&& cma : accumulators.getBuffer())
        REQUIRE(cma.getAverage() == 3);
//    REQUIRE(average.getAverage() == 0);
}

TEST_CASE ("Cumulative Buffer Average", "[Buffer Accumulator]")
{
    CumulativeAverage<double> average{};
    REQUIRE(average.updateAverage(1) == 1);
    REQUIRE(average.updateAverage(2) == 1.5);
    REQUIRE(average.updateAverage(3) == 2);
    average.reset();
    REQUIRE(average.getAverage() == 0);
}