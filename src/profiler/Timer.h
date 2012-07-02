#pragma once

#include "profiler/Platform.h"
#include "profiler/Utilities.h"

namespace profiling
{

    /*
    =============
    Timer
    =============
    */
#pragma pack(push,1)
    struct PROFILER_API Timer {
        Timer() { Reset(); }

        inline bool IsEmpty() const { return ticks == 0; }
        inline bool IsPaused() const { return paused; }
        inline void Unpause( u64 curticks ) { started = curticks; paused = false; }
        inline void Unpause() { Unpause( getticks() ); }
        inline void Pause( u64 curticks ) { ticks += ( curticks - started ); paused = true; }
        inline void Pause() { Pause( getticks() ); }		
        inline void Start() { ++calls; started = getticks(); }
        inline void Stop() { ticks += ( getticks() - started ); }
        inline void Reset() { ticks = started = calls = 0; paused = false; }
        inline void SoftStop() { if ( !paused ) { u64 t = getticks(); ticks += ( t - started ); started = t; } }
        inline void SoftReset() { ticks = 0; calls = 1; started = getticks(); }

        template< class type > static f64 ms( const type &t ) { return f64( t ) / 1000000.0; }
        f64 millicycles() { return ms( ticks ); }
        f64 currentmillicycles() { return ms( ticks + ( getticks() - started ) ); }
        f64 avg() { return average( ticks, calls ); }
        f64 avgms() { return ms( average( ticks, calls ) ); }

        void operator+= ( const Timer &b ) {
            ticks += b.ticks;
            calls += b.calls;
        }

        static inline u64 getticks_serial() {
#if defined(__GNUC__)
            asm volatile("cpuid" : : : "%eax", "%ebx", "%ecx", "%edx" );
#else
            __asm cpuid;
#endif
            return getticks();			
        }

#if defined(__GNUC__)
        static inline u64 getticks() {
            u32 __a,__d;
            asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
            return ( u64(__a) | u64(__d) << 32 );
        }
#elif defined(__ICC) || defined(__ICL)
        static inline u64 getticks() { return _rdtsc(); }
#else
        static inline u64 getticks() { __asm { rdtsc }; }
#endif

        u64 ticks, started;
        u32 calls;
        bool paused;
    };
#pragma pack(pop)

    struct PROFILER_API ScopedTimer {
        ScopedTimer( profiling::Timer &t ) : mTimer(t) { mTimer.Start(); }
        ~ScopedTimer() { mTimer.Stop(); }
    protected:
        profiling::Timer &mTimer;
    };

}