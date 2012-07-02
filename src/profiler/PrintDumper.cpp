#include "profiler/PrintDumper.h"

#include "profiler/Caller.h"

namespace profiling
{
    void PrintDumper::Init() {
    }

    void PrintDumper::GlobalInfo( u64 rawCycles ) {
        printf( "> Raw run time %.2f mcycles\n", Timer::ms( rawCycles ) );
    }

    void PrintDumper::ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead ) {
        printf( "> Total calls " PRINTFU64() ", per call overhead %.0f cycles, rdtsc overhead %.0f cycles, estimated overhead %.2f mcycles\n\n",
            totalCalls, timerOverhead, rdtscOverhead, Timer::ms( timerOverhead * totalCalls ) );
    }

    void PrintDumper::PrintThread( Caller *root ) {
        root->Print();
        printf( "\n\n" );
    }

    void PrintDumper::PrintAccumulated( Caller *accumulated ) {
        accumulated->PrintTopStats( 50 );
    }

    void PrintDumper::Finish() {
    }
}