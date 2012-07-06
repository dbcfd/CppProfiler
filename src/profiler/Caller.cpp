#include "profiler/Caller.h"
#include "profiler/Profiler.h"
#include "profiler/HtmlDumper.h"

namespace profiling
{
    void Caller::Format::operator()( Caller *item, bool islast ) const {
        u64 ticks = item->mTimer.ticks;
        f64 ms = Timer::ms( ticks );
        printf( "%s %.2f mcycles, %d calls, %.0f cycles avg, %.2f%%: %s\n", 
            mPrefix, ms, item->mTimer.calls, item->mTimer.avg(), average( ticks * 100, Profiler()->globalDuration ), item->mName.c_str() );
    }

    void Caller::FormatHtml::operator()( Caller *item ) const {
        fprintf( mFile, "\t<tr %s><td><table class=\"tree\"><tr>", !item->GetParent() ? "style=\"" css_thread_style "\"": "class=\"h\"" );
        for ( u32 i = 0; i < mPrefix.Size(); i++ )
            fprintf( mFile, "<td>%s</td>", mPrefix[i] );
        u64 ticks = item->mTimer.ticks;
        f64 ms = Timer::ms( ticks );
        f64 childms = Timer::ms( item->mChildTicks ), selfms = ( ms - childms ), avg = item->mTimer.avgms(), selfavg = average( selfms, item->mTimer.calls );
        f64 globalPct = (f64)(ticks*100) / (f64)Profiler()->globalDuration;
        if ( !item->GetParent() )
        {
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\">%u</td><td class=\"number\">%0.4f (%3.0f%%)</td><td class=\"number\">%0.4f</td><td class=\"number\">%0.4f</td><td class=\"number\">%0.4f</td></tr>\n", 
            item->mName.c_str(), 
            item->mTimer.calls,
            ms,
            globalPct,
            avg,
            selfms,
            selfavg
            );
        }
        else
        {
            Caller* rootCaller = Profiler()->getRootCaller();
            if(rootCaller == item)
            {
                globalPct = 100.0;
            }
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\" style=\"background-color:%s\">%u</td><td class=\"number\" style=\"background-color:%s\">%0.4f (%3.0f%%)</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td></tr>\n", 
            item->mName.c_str(), 
            rootCaller->maxStats.color( Max::Calls, item->mTimer.calls ),  item->mTimer.calls,
            rootCaller->maxStats.color( Max::Ms, ms ), ms,
            globalPct,
            rootCaller->maxStats.color( Max::Avg, avg ), avg,
            rootCaller->maxStats.color( Max::SelfMs, selfms ), selfms,
            rootCaller->maxStats.color( Max::SelfAvg, selfavg ), selfavg
            );
        }
    }

    void Caller::FormatHtmlTop::operator()( Caller *item, bool islast ) const {
        fprintf( mFile, "\t<tr %s><td><table class=\"tree\"><tr>", !item->GetParent() ? "style=\"" css_thread_style "\"": "class=\"h\"" );
        fprintf( mFile, "<td>|_&nbsp;</td>");
        u64 ticks = item->mTimer.ticks;
        f64 ms = Timer::ms( ticks ), avg = item->mTimer.avgms();
        f64 globalPct = (f64)(ticks) / (f64)Profiler()->globalDuration;
        if ( item->GetParent() ) {
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\">%u</td><td class=\"number\">%0.4f (%.0f%%)</td><td class=\"number\">%0.4f</td></tr>\n", 
                item->mName.c_str(), 
                item->mTimer.calls,
                ms,
                globalPct,
                avg
                );
        } else {
            Caller* rootCaller = Profiler()->getRootCaller();
            if(rootCaller == item)
            {
                globalPct = 100.0;
            }
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\" style=\"background-color:%s\">%u</td><td class=\"number\" style=\"background-color:%s\">%0.4f (%.0f%%)</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td></tr>\n", 
                item->mName.c_str(), 
                rootCaller->maxStats.color( Max::Calls, item->mTimer.calls ),  item->mTimer.calls,
                rootCaller->maxStats.color( Max::Ms, ms ), ms,
                globalPct,
                rootCaller->maxStats.color( Max::Avg, avg ), avg
                );
        }
    }


    // we're guaranteed to be null because of calloc. ONLY create Callers with "new"!
    Caller::Caller( const std::string& name, Caller *parent ) : mFormatter(64), mHtmlFormatter(64), mName(name) { 
        mParent = parent;
    }

    Caller::~Caller() { 
        Reset();
    }

    void Caller::CopyToListNonEmpty( Buffer<Caller *> &list ) {
        list.Clear();

        for(Buckets::const_iterator iter = mBuckets.begin(); iter != mBuckets.end(); ++iter)
        {
            Caller* caller = iter->second;
            if(!caller->GetTimer().IsEmpty())
            {
                list.Push(caller);
            }
        }
    }

    Caller *Caller::Find( const std::string& name ) {
        Caller* ret = 0;
        if(name == mName)
        {
            ret = this;
        }
        else
        {
            Buckets::const_iterator iter = mBuckets.find(name);
            if(mBuckets.end() != iter)
            {
                ret = iter->second;
            }
        }
        return ret;
    }

    Caller* Caller::Create(const std::string& name)
    {
        Caller* caller = new Caller( name, this );
        mBuckets[name] = caller;
        return caller;
    }

    inline Caller *Caller::GetParent() { 
        return mParent; 
    }

    Timer & Caller::GetTimer() { 
        return mTimer;
    }

    const std::string& Caller::GetName() const {
        return mName;
    }

    bool Caller::IsActive() const {
        return mActive;
    }

    void Caller::Print( u32 indent, bool islast) {
        Buffer<Caller *> children( mNumChildren );
        CopyToListNonEmpty( children );

        mFormatter.EnsureCapacity( indent + 3 );
        char *fmt = ( &mFormatter[indent] );

        if ( indent ) {
            fmt[-2] = ( islast ) ? ' ' : '|';
            fmt[-1] = ( islast ) ? '\\' : ' ';
        }
        fmt[0] = ( children.Size() ) ? '+' : '-';
        fmt[1] = ( '-' );
        fmt[2] = ( 0 );

        Format(mFormatter.Data())( this, islast );

        if ( indent && islast )
            fmt[-2] = fmt[-1] = ' ';

        if ( children.Size() ) {
            children.Sort( compare::Ticks() );
            children.ForEach( foreach::Printer(indent+2) );
        }
    }

    void Caller::PrintHtml( FILE *f, u32 indent, bool islast ) {
        Buffer<Caller *> children( mNumChildren );
        CopyToListNonEmpty( children );

        for(u32 i = 1; i < indent; ++i)
        {
            mHtmlFormatter.Push("&nbsp;");
        }

        if ( !indent ) {
            mHtmlFormatter.Push( "[]" );
        } else if ( children.Size() ) {
            mHtmlFormatter.Push( ( ( islast ) || ( children.Size() == 0 ) ) ? "-" : "+" );
        } else {
            mHtmlFormatter.Push( "|_&nbsp;");
        }

        FormatHtml(this, f, mHtmlFormatter)( this );

        mHtmlFormatter[indent] = ( islast || !indent ) ? "&nbsp;" : "|";	

        if ( children.Size() ) {
            children.Sort( compare::Ticks() );
            children.ForEach( foreach::PrinterHtml(f,indent+1) );
        }

        mHtmlFormatter.Pop();		
    }

    void Caller::SaveTopStats( u32 nitems, std::stringstream& stream ) {
        nitems = ( nitems > mNumChildren ) ? mNumChildren : nitems;
        //stream << "\ntop " << nitems << " functions (self time)\n";
        Buffer<Caller *> sorted( mNumChildren );
        CopyToListNonEmpty( sorted );
        sorted.Sort( compare::SelfTicks() );
        sorted.ForEach( Format(">"), nitems );
    }

    void Caller::PrintTopStats( u32 nitems ) {
        nitems = ( nitems > mNumChildren ) ? mNumChildren : nitems;
        printf( "\ntop %d functions (self time)\n", (u32 )nitems );
        Buffer<Caller *> sorted( mNumChildren );
        CopyToListNonEmpty( sorted );
        sorted.Sort( compare::SelfTicks() );
        sorted.ForEach( Format(">"), nitems );
    }

    void Caller::Reset() {
        for(Buckets::iterator iter = mBuckets.begin(); iter != mBuckets.end(); ++iter)
        {
            delete iter->second;
        }
        mBuckets.clear();
        mTimer.Reset();			
    }

    void Caller::SetActive( bool active ) {
        mActive = active;
    }

    void Caller::SetParent( Caller *parent ) {
        mParent = parent;
    }

    void Caller::SoftReset() {
        mTimer.SoftReset();
        ForEach( foreach::SoftReset() );
    }

    void Caller::Start() {
        mTimer.Start();
    }

    void Caller::Stop() {
        mTimer.Stop();
    }

    void* Caller::operator new ( size_t size ) {
        return calloc( size, 1 );
    }

    void Caller::operator delete ( void *p ) { 
        free( p );
    }
}