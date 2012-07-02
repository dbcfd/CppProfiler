#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(PROFILER_BUILD)
#if defined(PROFILER_LINK_STATICALLY)
#define PROFILER_API
#else
#define PROFILER_API __declspec(dllexport)
#endif
#else
#if defined(PROFILER_LINK_STATICALLY)
#define PROFILER_API
#else
#define PROFILER_API __declspec(dllimport)
#endif
#endif

#undef threadlocal
#define threadlocal __declspec(thread)

#define PRINTFU64() "%I64u"
#define PATHSLASH() '\\'

namespace profiling
{
	/*
	=============
	Types that won't conflict with the rest of the system
	=============
	*/
	typedef float f32;
	typedef double f64;
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	#if defined(_MSC_VER)
		typedef unsigned __int64 u64;
	#else
		typedef unsigned long long u64;
	#endif
	
}