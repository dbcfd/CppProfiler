#pragma once

#include "Profiler/Platform.h"

namespace profiling
{
    struct Max {
        enum f64Enum { SelfMs = 0, Ms, Avg, SelfAvg, f64Enums };
        enum u64Enum { Calls = 0, TotalCalls, u64Enums };

        void reset();

        void check( u64Enum e, u64 u );
        void check( f64Enum e, f64 f );

        const char *color( u64Enum e, u64 u ) const;
        const char *color( f64Enum e, f64 f ) const;

        const u64 &operator() ( u64Enum e ) const;
        const f64 &operator() ( f64Enum e ) const;

    protected:
        u64 u64fields[u64Enums];
        f64 f64fields[f64Enums];
    };
}