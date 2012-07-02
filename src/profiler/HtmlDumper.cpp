#include "profiler/HtmlDumper.h"

#include "profiler/Buffer.h"
#include "profiler/Caller.h"
#include "profiler/Profiler.h"

#include <time.h>

namespace profiling
{
    HtmlDumper::HtmlDumper(){

    }
    void HtmlDumper::Init() {
        Caller* rootCaller = Profiler()->getRootCaller();
        rootCaller->mColors.clear();
        rootCaller->mColors.push( ColorF( 255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f ), 0.00f );
        //Caller::mColors.push( ColorF( 255.0f/255.0f, 212.0f/255.0f, 129.0f/255.0f ), 0.50f );
        rootCaller->mColors.push( ColorF( 255.0f/255.0f,  203.0f/255.0f,  203.0f/255.0f ), 0.20f );
        rootCaller->mColors.push( ColorF( 255.0f/255.0f,  128.0f/255.0f,  128.0f/255.0f ), 1.00f );

        time_t now;
        time( &now );
        tm now_tm;
        localtime_s( &now_tm, &now );
        strftime( timeFormat, 255, "%Y%m%d_%H%M%S", &now_tm );
        _snprintf_s( fileFormat, 255, "%s-profile-%s.html", Profiler()->programName ? Profiler()->programName : "no-info-given", timeFormat );
        strftime( timeFormat, 255, "%#c", &now_tm );			
        fopen_s(&f, fileFormat, "w" );

        rootCaller->mHtmlFormatter.Clear();
        fputs(
            "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
            "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\" class=\"\">\n"
            "<head>\n"
            "	<style type=\"text/css\">\n"
            "		body {font-family: arial;font-size: 11px;}\n"
            "		table {padding: 0px;margin: 0px;border-spacing: 0pt 0pt;}\n"
            "		table.tree td {padding: 0px; margin: 0px;}\n"
            "		tr {padding: 0px;margin: 0px;}\n"
            "		tr.h:hover {background-color: #EEEEEE; color:blue;}\n"
            "		tr.header td { padding-left: 8px; padding-right:8px; border-right:1px solid " css_outline_color "; border-top:1px solid " css_outline_color "; }\n"
            "		tr.header td.left { border-left:1px solid " css_outline_color "; }\n"
            "		tr.header td.right { border-right:1px solid " css_outline_color "; }\n"
            "		tr.spacer {height: 24px;}\n"
            "		td {padding: 0px;padding-left:3px;padding-right:3px;margin: 0px;}\n"
            "		td.text {text-align: left;}\n"
            "		td.number {text-align: right;}\n"
            "		div.overall { background-color: #F0F0F0; width: 98%; color: #A31212; font-size: 16px; padding: 5px; padding-left: 20px; margin-bottom: 15px; }\n"
            "		div.thread { margin-bottom: 15px; }\n"
            "		div.overall td.title { padding-left: 10px; font-weight: bold; }\n"
            "	</style>\n"
            "</head>\n"
            "<body>\n\n",			
            f
            );
    }

    void HtmlDumper::GlobalInfo( u64 rawCycles ) {
        fputs( "<div class=\"overall\"><table>", f );
        if ( Profiler()->programName ) {
            fprintf( f, "<tr><td class=\"title\">Command Line: </td><td>%s", Profiler()->programName );
            if ( Profiler()->commandLine )
                fprintf( f, " %s", Profiler()->commandLine );
            fputs( "</td></tr>", f );
        }
        fprintf( f, "<tr><td class=\"title\">Date: </td><td>%s</td></tr><tr><td class=\"title\">Raw run time: </td><td>%.2f mcycles</td></tr>\n", 
            timeFormat, Timer::ms( rawCycles ) );
    }

    void HtmlDumper::ThreadsInfo( u64 totalCalls, f64 timerOverhead, f64 rdtscOverhead ) {
        fprintf( f, "<tr><td class=\"title\">Total calls: </td><td>" PRINTFU64() "</td></tr>\n", totalCalls );
        fprintf( f, "<tr><td class=\"title\">rdtsc overhead: </td><td>%.0f cycles</td></tr>\n", rdtscOverhead );
        fprintf( f, "<tr><td class=\"title\">Per call overhead: </td><td>%.0f cycles</td></tr>\n", timerOverhead );
        fprintf( f, "<tr><td class=\"title\">Estimated overhead: </td><td>%.4f mcycles</td></tr>\n", Timer::ms( totalCalls * timerOverhead ) );
        fprintf( f, "</table></div>\n" );
    }

    void HtmlDumper::PrintThread( Caller *root ) {
        fputs( "<div class=\"thread\"><table>\n", f );
        fputs( css_title_row, f );
        root->PrintHtml( f );
        fputs( "</table></div>\n", f );
    }

    void HtmlDumper::PrintAccumulated( Caller *accumulated ) {
        fputs( "<div class=\"thread\"><table>\n", f );
        fputs( css_totals_row, f );
        fputs( "<tr style=\"" css_thread_style "\"><td><table><tr><td>Functions sorted by self time</td></tr></table></td><td></td><td></td><td></td></tr>\n", f );
        Buffer<Caller *> sorted;
        accumulated->CopyToListNonEmpty( sorted );
        sorted.ForEach( Caller::foreach::UpdateTopMaxStats(accumulated) );
        sorted.Sort( Caller::compare::SelfTicks() );		
        sorted.ForEach( Caller::FormatHtmlTop(accumulated, f) );
        fputs( "</table></div>\n", f );
    }

    void HtmlDumper::Finish() {
        fputs( "</div>\n", f );
        fputs( "</body></html>", f );
        fclose( f );
    }

}
