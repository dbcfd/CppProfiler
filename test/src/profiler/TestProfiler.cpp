#include "profiler/Profiler.h"

#pragma warning(disable:4251)
#include <gtest/gtest.h>

double testValues[] = {
#include "TestValues.h"
};

void multiplyFunction(int maxValueToProcess)
{
    PROFILE_SCOPED();
    double lastValue = 1.0;
    for(int i = 0; i < 1000; ++i)
    {
        double value = testValues[i] * lastValue;
        lastValue = testValues[i];
    }
}

void divideFunction(int maxValueToProcess)
{
    PROFILE_SCOPED();
    double lastValue = 1.0;
    for(int i = 0; i < 1000; ++i)
    {
        double value = testValues[i] / lastValue;
        lastValue = testValues[i];
    }
}

TEST(PROFILER_TEST, NO_THREADING_TEST)
{
    int argc = 1;
    char* argv[] = {"TestProfiler.exe"};

    PROFILER_DETECTARGS(argc, argv);

    PROFILE_SCOPED();

    multiplyFunction(1000);
    divideFunction(1000);

    multiplyFunction(500);
    divideFunction(500);

    std::string output = PROFILER_DUMPSTRING();

    EXPECT_TRUE(output.size() > 0);

    PROFILER_DUMPHTML();
}