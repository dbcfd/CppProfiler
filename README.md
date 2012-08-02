# C++ Profiler

Based on Andrew's High Performance C++ Profiler (http://floodyberry.wordpress.com/2009/10/07/high-performance-cplusplus-profiling/), this is an inline profiler.

## Build Instructions
 * Install CMake (http://cmake.org/cmake/resources/software.html). 
 * Open a command prompt (unless you're using the CMake gui)
 * Switch to the location you have the Profiler source code at.
 * cd to the build directory
 * Run CMake

If using Visual Studio a standard cmake command is 
>> cmake .. -G"Visual Studio 9 2008" -DLINK_STATICALLY:BOOL=ON -DCMAKE_BUILD_TYPE:STRING=Release

This will build with Visual Studio 2008, and link the library statically. It will also build in Release mode.

## Basic Usage
Set compiler flag ENABLE_PROFILER for any build type you wish to profile. If using inline or linking statically, set USE_PROFILER_STATICALLY.

Call PROFILER_DETECTARGS at the start of your programs.

Call PROFILE_SCOPED or PROFILE_SCOPED_DESC at the beginning of any scoped block you wish to profile.

Call PROFILER_DUMPHTML, PROFILER_DUMPSTRING, or PROFILER_DUMP at the end to receive results.

