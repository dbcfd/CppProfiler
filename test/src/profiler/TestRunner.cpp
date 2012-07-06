#include <iostream>
#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>
#include "profiler/Profiler.h"

int main(int argc, char **argv) {
    PROFILER_DETECTARGS(argc, argv);
    std::cout << "Running main() from TestRunner.cpp\n";

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    
    PROFILER_DUMPHTML();

    return ret;
}