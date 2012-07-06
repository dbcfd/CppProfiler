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
    {
        PROFILE_SCOPED();

        multiplyFunction(1000);
        divideFunction(1000);

        multiplyFunction(500);
        divideFunction(500);
    }

    std::string output = PROFILER_DUMPSTRING();

    EXPECT_TRUE(output.size() > 0);
}

#ifdef WIN32

#include <vector>
#include <windows.h>

typedef void (*ComputeFunctionMethod)(int);

struct ComputeFunction {
    ComputeFunction()
    {

    }
    ComputeFunction(ComputeFunctionMethod _method, const std::vector<int>& args) : method(_method), arguments(args)
    {

    }
    ComputeFunctionMethod method;
    std::vector<int> arguments;
};

DWORD WINAPI RunFunctionInThread(LPVOID params)
{
    PROFILE_SCOPED();
    ComputeFunction* func = (ComputeFunction*)params;
    for(std::vector<int>::const_iterator iter = func->arguments.begin();
        iter != func->arguments.end();
        ++iter)
    {
        (*func->method)(*iter);
    }
    return 0;
}

TEST(PROFILER_TEST, THREADING_TEST)
{
    PROFILE_SCOPED();
    DWORD threadMultiplyId, threadDivideId;
    HANDLE handles[2];

    std::vector<int> arguments;
    arguments.push_back(1000);
    arguments.push_back(500);
    arguments.push_back(750);
    arguments.push_back(900);
    arguments.push_back(600);
    ComputeFunction multiply(&multiplyFunction, arguments);
    ComputeFunction divide(&divideFunction, arguments);

    handles[0] = CreateThread(
        NULL,
        0,
        RunFunctionInThread,
        &multiply,
        0,
        &threadMultiplyId);
    handles[1] = CreateThread(
        NULL,
        0,
        RunFunctionInThread,
        &divide,
        0,
        &threadDivideId);
    WaitForMultipleObjects(2, handles, TRUE, INFINITE);
}

#endif