#include "profiler/Caller.h"
#include "profiler/Profiler.h"
#include "profiler/HtmlDumper.h"

namespace profiling
{
    void Caller::Format::operator()( Caller *item, bool islast ) const {
        u64 ticks = item->mTimer.ticks;
        f64 ms = Timer::ms( ticks );
        printf( "%s %.2f mcycles, %d calls, %.0f cycles avg, %.2f%%: %s\n", 
            mPrefix, ms, item->mTimer.calls, item->mTimer.avg(), average( ticks * 100, Profiler()->globalDuration ), item->mName );
    }

    void Caller::FormatHtml::operator()( Caller *item ) const {
        fprintf( mFile, "\t<tr %s><td><table class=\"tree\"><tr>", !item->GetParent() ? "style=\"" css_thread_style "\"": "class=\"h\"" );
        for ( u32 i = 0; i < mPrefix.Size(); i++ )
            fprintf( mFile, "<td>%s</td>", mPrefix[i] );
        u64 ticks = item->mTimer.ticks;
        f64 ms = Timer::ms( ticks ), globalpct = average( ticks * 100, Profiler()->globalDuration );
        f64 childms = Timer::ms( item->mChildTicks ), selfms = ( ms - childms ), avg = item->mTimer.avgms(), selfavg = average( selfms, item->mTimer.calls );
        if ( !item->GetParent() )
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\">%u</td><td class=\"number\">%0.4f (%3.0f%%)</td><td class=\"number\">%0.4f</td><td class=\"number\">%0.4f</td><td class=\"number\">%0.4f</td></tr>\n", 
            item->mName, 
            item->mTimer.calls,
            ms,
            globalpct,
            avg,
            selfms,
            selfavg
            );
        else
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\" style=\"background-color:%s\">%u</td><td class=\"number\" style=\"background-color:%s\">%0.4f (%3.0f%%)</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td></tr>\n", 
            item->mName, 
            mRoot->maxStats.color( mRoot, Max::Calls, item->mTimer.calls ),  item->mTimer.calls,
            mRoot->maxStats.color( mRoot, Max::Ms, ms ), ms,
            globalpct,
            mRoot->maxStats.color( mRoot, Max::Avg, avg ), avg,
            mRoot->maxStats.color( mRoot, Max::SelfMs, selfms ), selfms,
            mRoot->maxStats.color( mRoot, Max::SelfAvg, selfavg ), selfavg
            );
    }

    void Caller::FormatHtmlTop::operator()( Caller *item, bool islast ) const {
        fprintf( mFile, "\t<tr %s><td><table class=\"tree\"><tr>", !item->GetParent() ? "style=\"" css_thread_style "\"": "class=\"h\"" );
        fprintf( mFile, "<td>|_&nbsp;</td>");
        u64 ticks = item->mTimer.ticks;
        f64 ms = Timer::ms( ticks ), globalpct = average( ticks * 100, Profiler()->globalDuration ), avg = item->mTimer.avgms();
        if ( !item->GetParent() ) {
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\">%u</td><td class=\"number\">%0.4f (%.0f%%)</td><td class=\"number\">%0.4f</td></tr>\n", 
                item->mName, 
                item->mTimer.calls,
                ms,
                globalpct,
                avg
                );
        } else {
            fprintf( mFile, "<td class=\"text\">%s</td></tr></table></td><td class=\"number\" style=\"background-color:%s\">%u</td><td class=\"number\" style=\"background-color:%s\">%0.4f (%.0f%%)</td><td class=\"number\" style=\"background-color:%s\">%0.4f</td></tr>\n", 
                item->mName, 
                mRoot->maxStats.color( mRoot,  Max::Calls, item->mTimer.calls ),  item->mTimer.calls,
                mRoot->maxStats.color( mRoot, Max::Ms, ms ), ms,
                globalpct,
                mRoot->maxStats.color( mRoot, Max::Avg, avg ), avg
                );
        }
    }


    // we're guaranteed to be null because of calloc. ONLY create Callers with "new"!
    Caller::Caller( const char *name, Caller *parent ) : mFormatter(64), mHtmlFormatter(64) { 
        mName = name;
        mParent = parent;
        Resize( 2 ); // mBuckets must always exist and mBucketCount >= 2!
    }

    Caller::~Caller() { 
        ForEach( foreach::Deleter() );
        free( mBuckets );
    }

    void Caller::CopyToListNonEmpty( Buffer<Caller *> &list ) {
        list.Clear();

        for ( u32 i = 0; i < mBucketCount; ++i )
            if ( mBuckets[ i ] && !mBuckets[ i ]->GetTimer().IsEmpty() )
                list.Push( mBuckets[ i ] );
    }

    Caller *Caller::Find( const char *name ) {
        Caller* ret = 0;
        if(name == mName)
        {
            ret = this;
        }
        else
        {
            u32 index = ( GetBucket( name, mBucketCount ) ), mask = ( mBucketCount - 1 );
            for ( Caller *caller = mBuckets[index]; caller; caller = mBuckets[index & mask] ) {
                if ( caller->mName == name )
                {
                    ret = caller;
                }

                index = ( index + 1 );
            }
        }
        return ret;
    }

    Caller* Caller::Create(const char* name)
    {
        EnsureCapacity( ++mNumChildren );
        Caller *&slot = FindEmptyChildSlot( mBuckets, mBucketCount, name );
        slot = new Caller( name, this );
        return slot;
    }

    inline Caller *Caller::GetParent() { 
        return mParent; 
    }

    Timer & Caller::GetTimer() { 
        return mTimer;
    }

    const char * Caller::GetName() const {
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

        if ( !indent ) {
            mHtmlFormatter.Push( "[]" );
        } else if ( children.Size() ) {
            mHtmlFormatter.Push( ( ( islast ) || ( children.Size() == 1 ) ) ? "-" : "+" );
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

    void Caller::Resize( u32 new_size ) {
        new_size = ( new_size < mBucketCount ) ? mBucketCount << 1 : nextpow2( new_size - 1 );
        Caller **new_buckets = (Caller **)calloc( new_size, sizeof( Caller* ) );
        ForEach( foreach::AddToNewBuckets( new_buckets, new_size ) );

        free( mBuckets );
        mBuckets = ( new_buckets );
        mBucketCount = ( new_size );
    }

    void Caller::Reset() {
        ForEach( foreach::Deleter() );
        zeroarray( mBuckets, mBucketCount );
        mNumChildren = ( 0 );
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

    Caller *& Caller::FindEmptyChildSlot( Caller **buckets, u32 bucket_count, const char *name ) {
        u32 index = ( GetBucket( name, bucket_count ) ), mask = ( bucket_count - 1 );
        Caller **caller = &buckets[index];
        for ( ; *caller; caller = &buckets[index & mask] )
            index = ( index + 1 );
        return *caller;
    }

    u32 Caller::GetBucket( const char *name, u32 bucket_count ) {
        return u32( ( ( (size_t )name >> 5 ) /* * 2654435761 */ ) & ( bucket_count - 1 ) );
    }

    void Caller::EnsureCapacity( u32 capacity ) {
        if ( capacity < ( mBucketCount / 2 ) )
            return;
        Resize( capacity );
    }
}