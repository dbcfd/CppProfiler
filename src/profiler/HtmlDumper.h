#pragma once

#include "profiler/Platform.h"

#define css_outline_color "#848484"
#define css_thread_style "background-color:#EEEEEE;margin-top:8px;"
#define css_title_row "<tr class=\"header\"><td class=\"left\">Function</td><td>Calls</td><td>MCycles</td><td>Avg</td><td>Self MCycles</td><td class=\"right\">Self Avg</td></tr>\n"
#define css_totals_row "<tr class=\"header\"><td class=\"left\">Function</td><td>Calls</td><td>Self MCycles</td><td class=\"right\">Self Avg</td></tr>\n"

namespace profiling
{
    class Caller;

    class HtmlDumper {
    public:
        HtmlDumper();

        void Init();

        void GlobalInfo( u64 rawCycles );

        void ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead );

        void PrintThread( Caller *root );

        void PrintAccumulated( Caller *accumulated );

        void Finish();

    protected:
        FILE *f;
        char timeFormat[256];
        char fileFormat[256];

    };

}