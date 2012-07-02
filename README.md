# C++ Profiler

Based on Andrew's High Performance C++ Profiler (http://floodyberry.wordpress.com/2009/10/07/high-performance-cplusplus-profiling/), this is an inline profiler.

## Basic Usage
Set compiler flag ENABLE_PROFILER for any build type you wish to profile. If using inline or linking statically, set USE_PROFILER_STATICALLY.

Call PROFILER_DETECTARGS at the start of your programs.

Call PROFILE_SCOPED or PROFILE_SCOPED_DESC at the beginning of any scoped block you wish to profile.

Call PROFILER_DUMPHTML, PROFILER_DUMPSTRING, or PROFILER_DUMP at the end to receive results.

