#pragma once

#include <map>

#include "profiler/Platform.h"
#include "profiler/Buffer.h"
#include "profiler/CASLock.h"

#undef noinline
#undef fastcall

#define PROFILE_CONCAT( a, b ) a "/" b

#define noinline __declspec(noinline)
#define fastcall __fastcall

#define PROFILE_FUNCTION() __FUNCSIG__

#if defined(ENABLE_PROFILER)
// thread
#define PROFILE_THREAD_START_RAW( text )   profiling::Profiler()->threadenter( text );
#define PROFILE_THREAD_START()             PROFILE_THREAD_START_RAW( PROFILE_FUNCTION()  )
#define PROFILE_THREAD_START_DESC( desc )  PROFILE_THREAD_START_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

#define PROFILE_THREAD_SCOPED_RAW( text )  profiling::ScopedThread profiler##__LINE__ ( text );
#define PROFILE_THREAD_SCOPED()            PROFILE_THREAD_SCOPED_RAW( PROFILE_FUNCTION() )
#define PROFILE_THREAD_SCOPED_DESC( desc ) PROFILE_THREAD_SCOPED_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

#define PROFILE_THREAD_STOP()              profiling::Profiler()->threadexit();

// function
#define PROFILE_PAUSE()             profiling::Profiler()->pause();
#define PROFILE_UNPAUSE()           profiling::Profiler()->unpause();
#define PROFILE_PAUSE_SCOPED()      profiling::ScopedPause profilerpause##__LINE__;

#define PROFILE_START_RAW( text )   profiling::Profiler()->enter( text );
#define PROFILE_START()             PROFILE_START_RAW( PROFILE_FUNCTION()  )
#define PROFILE_START_DESC( desc )  PROFILE_START_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

#define PROFILE_SCOPED_RAW( text )  profiling::Scoped profiler##__LINE__ ( text );
#define PROFILE_SCOPED()            PROFILE_SCOPED_RAW( PROFILE_FUNCTION() )
#define PROFILE_SCOPED_DESC( desc ) PROFILE_SCOPED_RAW( PROFILE_CONCAT( PROFILE_FUNCTION(), desc ) )

#define PROFILE_STOP()              profiling::Profiler()->exit();

#define PROFILER_DETECTARGS(argc, argv) profiling::Profiler()->detect(argc, argv)
#define PROFILER_DUMP()                 profiling::Profiler()->dump()
#define PROFILER_DUMPHTML()             profiling::Profiler()->dumphtml()
#define PROFILER_DUMPSTRING()           profiling::Profiler()->dumpstring()
#else
#define PROFILE_THREAD_START_RAW( text )
#define PROFILE_THREAD_START()
#define PROFILE_THREAD_START_DESC( desc )

#define PROFILE_THREAD_SCOPED_RAW( text )
#define PROFILE_THREAD_SCOPED()
#define PROFILE_THREAD_SCOPED_DESC( desc )

#define PROFILE_THREAD_STOP()

#define PROFILE_PAUSE()
#define PROFILE_UNPAUSE()
#define PROFILE_PAUSE_SCOPED()

#define PROFILE_START_RAW( text )
#define PROFILE_START()
#define PROFILE_START_DESC( desc )

#define PROFILE_SCOPED_RAW( text )
#define PROFILE_SCOPED()
#define PROFILE_SCOPED_DESC( desc )

#define PROFILE_STOP()
#define PROFILER_DETECTARGS(argc, argv) 
#define PROFILER_DUMP()                
#define PROFILER_DUMPHTML()      
#define PROFILER_DUMPSTRING()           std::string()
#endif

namespace profiling {
    /*
    =============
    Interface functions
    =============
    */
    class Caller;
    struct ThreadInformation
    {
        ThreadInformation() : root(0), current(0), requireThreadLock(false)
        {

        }
        ThreadInformation(Caller* _root, const ThreadInformation& other) : root(_root), requireThreadLock(other.requireThreadLock)
        {

        }
        void lock()
        {
            threadLock.Acquire();
        }
        void unlock()
        {
            threadLock.Release();
        }
        Caller* root;
        CASLock threadLock;
        bool requireThreadLock;
        Caller* current;
    };

    class PROFILER_API ProfilerInterface {
    public:
        ProfilerInterface();
        ~ProfilerInterface();
        void detect( int argc, char *argv[] );
        void detect( int argc, const char *argv[] );
        void detect( const char *commandLine );
        std::string dumpstring();
        void dump();
        void dumphtml();
        void fastcall enter( const char *name );
        void exit();
        void pause();
        void unpause();
        void threadenter( const char *name );
        void threadexit();
        void reset();
        void pack(Caller* packer, Buffer<Caller *>& packedThreads);

        ThreadInformation& getThreadInformation();
        Caller* getRootCaller();

        u64 globalDuration;
        char* programName;
        char* commandLine;
        u64 globalStart;
        f64 timerOverhead;
        f64 rdtscOverhead;

        static ProfilerInterface* instance();

    private:
        void resetThreads();
        void enterThread( const char *name );
        void exitThread();
        void enterCaller( const char *name );
        void exitCaller();
        void pauseCaller();
        void unpauseCaller();
        void lock();
        void unlock();

        ThreadInformation* main;
        std::map<DWORD, ThreadInformation> threads;
        CASLock threadsLock;
        static ProfilerInterface* ProfilerPtr;
    };

    static ProfilerInterface* PROFILER_API Profiler()
    {
        return ProfilerInterface::instance();
    }

    struct PROFILER_API Scoped {
        Scoped( const char *name ) { PROFILE_START_RAW( name ) }
        ~Scoped() { PROFILE_STOP() }
    };

    struct PROFILER_API ScopedPause {
        ScopedPause() { PROFILE_PAUSE() }
        ~ScopedPause() { PROFILE_UNPAUSE() }
    };

    struct PROFILER_API ScopedThread {
        ScopedThread( const char *name ) { PROFILE_THREAD_START_RAW( name ); }
        ~ScopedThread() { PROFILE_THREAD_STOP() }
    };
}; // namespace Profiler
