#pragma once

#include "profiler/Platform.h"
#include "profiler/Max.h"
#include "profiler/Profiler.h"

namespace profiling 
{
    template< class Dumper >
    class DumpThreads
    {
    public:
        DumpThreads( Dumper& dumper ) {
            u64 rawDuration = ( Timer::getticks() - Profiler()->globalStart );

            Caller *accumulate = new Caller( "/Top Callers" ), *packer = new Caller( "/Thread Packer" );
            Caller* root = Profiler()->getRootCaller();
            Buffer<Caller *> packedThreads;

            dumper.Init();
            dumper.GlobalInfo( rawDuration );

            Profiler()->pack(packer, packedThreads);

            // do the pre-computations on the gathered threads
            Caller::ComputeChildTicks preprocessor(root, *accumulate );
            for ( u32 i = 0; i < packedThreads.Size(); i++ )
                preprocessor( packedThreads[i] );

            dumper.ThreadsInfo( root->maxStats( Max::TotalCalls ), Profiler()->timerOverhead, Profiler()->rdtscOverhead );

            // print the gathered threads
            u64 sumTicks = 0;
            for ( u32 i = 0; i < packedThreads.Size(); i++ ) {
                Caller *threadRoot = packedThreads[i];
                u64 threadTicks = threadRoot->GetTimer().ticks;
                sumTicks += threadTicks;
                dumper.PrintThread( threadRoot );
            }

            // print the totals, use the summed total of ticks to adjust percentages
            Profiler()->globalDuration = sumTicks;
            packedThreads.Clear();
            Profiler()->pack(accumulate, packedThreads);
            dumper.PrintAccumulated( accumulate );		
            dumper.Finish();

            delete accumulate;
            delete packer;
        }
    };
}