#include "profiler/Max.h"
#include "profiler/Caller.h"
#include "profiler/Profiler.h"

namespace profiling
{
    void Max::reset() {
        memset( this, 0, sizeof( *this ) );
    }

    void Max::check( u64Enum e, u64 u ) { if ( u64fields[e] < u ) u64fields[e] = u; if ( e == Calls ) u64fields[TotalCalls] += u; }
    void Max::check( f64Enum e, f64 f ) { if ( f64fields[e] < f ) f64fields[e] = f; }

    const char *Max::color( u64Enum e, u64 u ) const { 
        f32 value = f32(f64(u)/f64(u64fields[e]));
        return Profiler()->getRootCaller()->mColors.value( value ); 
    }
    const char *Max::color( f64Enum e, f64 f ) const { 
        f32 value = f32(f/f64fields[e]);
        return Profiler()->getRootCaller()->mColors.value( value ); 
    }

    const u64 &Max::operator() ( u64Enum e ) const { return u64fields[e]; }
    const f64 &Max::operator() ( f64Enum e ) const { return f64fields[e]; }
}