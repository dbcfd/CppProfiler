#pragma once

#include "profiler/Platform.h"

#include <sstream>

namespace profiling
{
    class Caller;

    class StringDumper {
    public:
        StringDumper() {}
        void Init();
        void GlobalInfo( u64 rawCycles );

        void ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead );

        void PrintThread( Caller *root );

        void PrintAccumulated( Caller *accumulated );

        void Finish();

        std::string getOutput();
    private:
        std::stringstream output;
    };

}