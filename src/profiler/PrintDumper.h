#pragma once

#include "profiler/Platform.h"

namespace profiling 
{
    class Caller;

    class PrintDumper {
    public:
        PrintDumper() {}

        void Init();

        void GlobalInfo( u64 rawCycles );

        void ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead );

        void PrintThread( Caller *root );

        void PrintAccumulated( Caller *accumulated );

        void Finish();
    };

}