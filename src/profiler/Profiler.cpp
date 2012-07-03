#define __PROFILER_SMP__
#define __PROFILER_CONSOLIDATE_THREADS__

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define copystring _strdup
#include <windows.h>
#else
#define copystring strdup
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "profiler/Profiler.h"

#if defined(__ICC) || defined(__ICL)
#pragma warning( disable: 1684 ) // (size_t )name >> 5
#pragma warning( disable: 1011 ) // missing return statement at end of non-void function
#endif

#undef inline
#define inline __forceinline

#include "profiler/Buffer.h"
#include "profiler/Caller.h"
#include "profiler/CASLock.h"
#include "profiler/DumpThreads.h"
#include "profiler/HtmlDumper.h"
#include "profiler/PrintDumper.h"
#include "profiler/StringDumper.h"
#include "profiler/Timer.h"
#include "profiler/Utilities.h"

namespace profiling {
    ProfilerInterface* ProfilerInterface::ProfilerPtr = new ProfilerInterface();

    void detectByArgsRecast( int argc, const char** argv ) {
        const char *path = argv[0], *finalSlash = path, *iter = path;
        for ( ; *iter; ++iter )
            finalSlash = ( *iter == PATHSLASH() ) ? iter + 1 : finalSlash;
        if ( !*finalSlash )
            finalSlash = path;
        Profiler()->programName = copystring( finalSlash );

        size_t width = 0;
        for ( int i = 1; i < argc; i++ ) {
            size_t len = strlen( argv[i] );
            Profiler()->commandLine = (char *)realloc( Profiler()->commandLine, width + len + 1 );
            memcpy( Profiler()->commandLine + width, argv[i], len );
            Profiler()->commandLine[width + len] = ' ';
            width += len + 1;
        }
        if ( width )
            Profiler()->commandLine[width - 1] = '\x0';
    }

    void detectByArgs( int argc, char *argv[] ) {
        if(0 != argc) detectByArgsRecast(argc, const_cast<const char**>(argv));
    }

    void detectByArgs(int argc, const char* argv[]) {
        if(0 != argc) detectByArgsRecast(argc, argv);
    }

    void detectWinMain( const char *cmdLine ) {
        char path[1024], *finalSlash = path, *iter = path;
        GetModuleFileName( NULL, path, 1023 );
        for ( ; *iter; ++iter )
            finalSlash = ( *iter == PATHSLASH() ) ? iter + 1 : finalSlash;
        if ( !*finalSlash )
            finalSlash = path;
        Profiler()->programName = copystring( finalSlash );
        Profiler()->commandLine = copystring( cmdLine );
    }


    ProfilerInterface::ProfilerInterface() : programName(0), commandLine(0), globalStart(Timer::getticks()), main(0)
    {
        // get an idea of how long timer calls / rdtsc takes
        const u32 reps = 1000;
        timerOverhead = rdtscOverhead = 1000000;			
        for ( u32 tries = 0; tries < 20; tries++ ) {
            Timer t, t2;
            t.Start();
            for ( u32 i = 0; i < reps; i++ ) {
                t2.Start();
                t2.Stop();
            }
            t.Stop();
            f64 avg = f64(t2.ticks)/f64(reps);
            if ( avg < rdtscOverhead )
                rdtscOverhead = avg;
            avg = f64(t.ticks)/f64(reps);
            if ( avg < timerOverhead )
                timerOverhead = avg;
        }

        enterThread( "/Main" );

        ThreadInformation& threadInfo = getThreadInformation();
        main = &threadInfo;
    }

    ProfilerInterface::~ProfilerInterface()
    {
        free( programName );
        free( commandLine );
        for(std::map<DWORD,ThreadInformation>::iterator iter = threads.begin(); iter != threads.end(); ++iter)
        {
            ThreadInformation& threadInfo = iter->second;
            delete threadInfo.root;   
        }
    }

    Caller* ProfilerInterface::getRootCaller()
    {
        return main->root;
    }

    ThreadInformation& ProfilerInterface::getThreadInformation()
    {
        return threads[GetCurrentThreadId()];
    }
    void ProfilerInterface::lock()
    {
        threadsLock.Acquire();
    }
    void ProfilerInterface::unlock()
    {
        threadsLock.Release();
    }
    void ProfilerInterface::resetThreads() {
        globalStart = Timer::getticks();

        lock();

        for(std::map<DWORD, ThreadInformation>::iterator iter = threads.begin(); iter != threads.end(); ++iter)
        {
            ThreadInformation& threadInfo = iter->second;
            if(threadInfo.root->IsActive())
            {
                delete threadInfo.root;
                threadInfo.root = 0;
            }
            else
            {
                threadInfo.lock();
                threadInfo.root->SoftReset();
                threadInfo.unlock();
            }
        }

        unlock();
    }


    void ProfilerInterface::enterThread( const char *name ) {
        Caller *tmp = new Caller( name );

        lock();

        ThreadInformation& threadInfo = getThreadInformation();
        if(0 == main)
        {
            threadInfo.root = tmp;
        }
        else
        {
            threadInfo.root = main->root;
        }
        threadInfo.current = tmp;

        threadInfo.lock();
        tmp->Start();
        tmp->SetActive( true );
        threadInfo.unlock();

        unlock();
    }

    void ProfilerInterface::exitThread() {
        lock();

        ThreadInformation& threadInfo = getThreadInformation();

        threadInfo.lock();

        threadInfo.root->Stop();
        threadInfo.root->SetActive( false );
        threadInfo.current = 0;

        threadInfo.unlock();

        unlock();
    }

    void ProfilerInterface::enterCaller( const char *name ) {
        ThreadInformation& threadInfo = getThreadInformation();

        Caller *parent = threadInfo.current;
        if ( !parent )
        {
            char buffer[100];
            sprintf_s(buffer, "%lu", GetCurrentThreadId());
            enterThread(buffer);
        }
        else
        {
            Caller *active = parent->Find( name );
            if(0 == active)
            {
                threadInfo.lock();
                active = parent->Create(name);
                threadInfo.unlock();
            }
            active->Start();
            threadInfo.current = active;
        }
    }

    void ProfilerInterface::exitCaller() {
        ThreadInformation& threadInfo = getThreadInformation();

        Caller *active = threadInfo.current;
        if ( !active )
            return;

        active->Stop();
        threadInfo.lock();
        threadInfo.current = active->GetParent();
        threadInfo.unlock();
    }

    void ProfilerInterface::pauseCaller() {
        ThreadInformation& threadInfo = getThreadInformation();

        u64 curticks = Timer::getticks();
        Caller *iter = threadInfo.current;
        for ( ; iter; iter = iter->GetParent() )
            iter->GetTimer().Pause( curticks );
    }

    void ProfilerInterface::unpauseCaller() {
        ThreadInformation& threadInfo = getThreadInformation();

        u64 curticks = Timer::getticks();
        Caller *iter = threadInfo.current;
        for ( ; iter; iter = iter->GetParent() )
            iter->GetTimer().Unpause( curticks );
    }

    void ProfilerInterface::detect( int argc, const char *argv[] ) { detectByArgs( argc, argv ); }
    void ProfilerInterface::detect( int argc, char *argv[] ) { detectByArgs( argc, argv ); }
    void ProfilerInterface::detect( const char *commandLine ) { detectWinMain( commandLine ); }
    void ProfilerInterface::dump() { 
        PrintDumper dumper;
        DumpThreads<PrintDumper> dt( dumper ); 
    }
    std::string ProfilerInterface::dumpstring() { 
        StringDumper dumper;
        DumpThreads<StringDumper> dt( dumper ); 
        return dumper.getOutput();
    }
    void ProfilerInterface::dumphtml() { 
        HtmlDumper dumper;
        DumpThreads<HtmlDumper> dt ( dumper ); 
    }
    void fastcall ProfilerInterface::enter( const char *name ) { enterCaller( name ); }
    void ProfilerInterface::exit() { exitCaller(); }
    void ProfilerInterface::pause() { pauseCaller(); }
    void ProfilerInterface::unpause() { unpauseCaller(); }
    void ProfilerInterface::threadenter( const char *name ) { enterThread( name ); }
    void ProfilerInterface::threadexit() { exitThread(); }
    void ProfilerInterface::reset() { resetThreads(); }

    ProfilerInterface* ProfilerInterface::instance()
    {
        return ProfilerPtr;
    }

    void ProfilerInterface::pack(Caller* packer, Buffer<Caller *>& packedThreads)
    {
        lock();	

        for(std::map<DWORD, ThreadInformation>::iterator iter = threads.begin(); iter != threads.end(); ++iter)
        {
            ThreadInformation& threadInfo = iter->second;

            threadInfo.lock();

            // if the thread is no longer active, the lock won't be valid
            bool active = ( threadInfo.root->IsActive() );
            if ( active ) {
                // disable requiring our local lock in case the caller is in our thread, accumulate will try to set it otherwise
                threadInfo.requireThreadLock = false;
                for ( Caller *walk = threadInfo.current; walk; walk = walk->GetParent() )
                {
                    walk->GetTimer().SoftStop();
                }
            }

            // merge the thread into the packer object, will result in 1 caller per thread name, not 1 caller per thread instance
            Caller::foreach::Merger( packer ).addFrom( threadInfo.root );
            Caller *child = packer->Find( threadInfo.root->GetName() );
            if(0 != child)
            {
                // add the child to the list of threads to dump (use the active flag to indicate if it's been added)
                if ( !child->IsActive() ) {
                    packedThreads.Push( child );
                    child->SetActive( true );
                }

                if ( active ) {
                    threadInfo.requireThreadLock = true;
                }
            }

            threadInfo.unlock();
        }

        // working on local data now, don't need the threads lock any more
        unlock();	
    }

}; // namespace Profiler
