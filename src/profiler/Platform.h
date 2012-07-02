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