#include "profiler/StringDumper.h"
#include "profiler/Caller.h"
#include "profiler/Timer.h"

namespace profiling {

    void StringDumper::Init() {
    }

    void StringDumper::GlobalInfo( u64 rawCycles ) {
        output << "> Raw run time " << Timer::ms( rawCycles ) << " mcycles\n";
    }

    void StringDumper::ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead ) {
        output << "> Total calls " << totalCalls << ", per call overhead ";
        output << timerOverhead << " cycles, rdtsc overhead " << rdtscOverhead << " cycles, estimated overhead ";
        output << Timer::ms( timerOverhead * totalCalls ) << " mcycles\n\n";
    }

    void StringDumper::PrintThread( Caller *root ) {
        root->Print();
        output << "\n\n";
    }

    void StringDumper::PrintAccumulated( Caller *accumulated ) {
        accumulated->SaveTopStats( 50, output);
    }

    void StringDumper::Finish() {
    }

    std::string StringDumper::getOutput()
    {
        return output.str();
    }
}