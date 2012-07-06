#pragma once

#include "profiler/Platform.h"
#include "profiler/Buffer.h"
#include "profiler/Max.h"
#include "profiler/HtmlColorRamp.h"
#include "profiler/Timer.h"

#include <string>
#include <map>

namespace profiling
{
    /*
    =============
    Caller
    =============
    */
#pragma pack(push,1)
    class Caller {
    public:
        typedef std::map<std::string, Caller*> Buckets;

        struct foreach {
            // Merges a Caller with the root
            struct Merger {
                Merger( Caller *_mergeInto ) : mergeInto(_mergeInto) {}
                void addFrom( Caller *item ) { (*this)( item ); }
                void operator()( Caller *item ) {
                    Caller* child = mergeInto->Find(item->GetName());
                    if(0 == child)
                    {
                        child = mergeInto->Create(item->GetName());
                    }
                    child->GetTimer() += item->GetTimer();
                    item->ForEachNonEmpty( Merger( child ) );
                }
                Caller *mergeInto;
            };

            // Prints a Caller
            struct Printer {
                Printer( u32 indent ) : mIndent(indent) { }
                void operator()( Caller *item, bool islast ) const {
                    item->Print( mIndent, islast );
                }
                u32 mIndent;
            };

            struct PrinterHtml {
                PrinterHtml( FILE *f, u32 indent ) : mFile(f), mIndent(indent) { }
                void operator()( Caller *item, bool islast ) const {
                    item->PrintHtml( mFile, mIndent, islast );
                }
                FILE *mFile;
                u32 mIndent;
            };

            struct SoftReset { 
                void operator()( Caller *item ) { 
                    item->GetTimer().SoftReset();
                    item->ForEach( SoftReset() );
                } 
            };

            // Sums Caller's ticks 
            struct SumTicks {
                SumTicks() : sum(0) {}
                void operator()( Caller *item ) { 
                    sum += ( item->mTimer.ticks );
                }
                u64 sum;
            };

            struct UpdateTopMaxStats {
                UpdateTopMaxStats(Caller* _caller) : caller(_caller) { caller->maxStats.reset(); }
                void operator()( Caller *item, bool islast ) {
                    if ( !item->GetParent() )
                        return;
                    caller->maxStats.check( Max::Calls, item->mTimer.calls );
                    caller->maxStats.check( Max::Ms, Timer::ms( item->mTimer.ticks ) );
                    caller->maxStats.check( Max::Avg, item->mTimer.avgms() );
                }
                Caller* caller;
            };
        }; // foreach


        struct compare {
            struct Ticks { 
                bool operator()( const Caller *a, const Caller *b ) const { 
                    return ( a->mTimer.ticks > b->mTimer.ticks ); 
                } 
            };

            struct SelfTicks { 
                bool operator()( const Caller *a, const Caller *b ) const { 
                    return ( ( a->mTimer.ticks - a->mChildTicks ) > ( b->mTimer.ticks - b->mChildTicks ) ); 
                } 
            };

            struct Calls { 
                bool operator()( const Caller *a, const Caller *b ) const { 
                    return ( a->mTimer.calls > b->mTimer.calls ); 
                } 
            };
        }; // sort


        /*
        Since Caller.mTimer.ticks is inclusive of all children, summing the first level
        children of a Caller to Caller.mChildTicks is an accurate total of the complete
        child tree.

        mTotals is used to keep track of total ticks by Caller excluding children
        */
        struct ComputeChildTicks {
            ComputeChildTicks( Caller* _root, Caller &totals ) : mRoot(_root), mTotals(totals) { mRoot->maxStats.reset(); }
            void operator()( Caller *item ) {
                foreach::SumTicks sumchildren;
                item->ForEachByRefNonEmpty( sumchildren );
                item->mChildTicks = ( sumchildren.sum );

                u64 selfticks = ( item->mTimer.ticks >= item->mChildTicks ) ? ( item->mTimer.ticks - item->mChildTicks ) : 0;
                Caller* totalitem = mTotals.Find( item->mName );
                if(0 != totalitem)
                {
                    totalitem->mTimer.ticks += selfticks;
                    totalitem->mTimer.calls += item->mTimer.calls;
                    totalitem->SetParent( item->GetParent() );
                }

                // don't include the root node in the max stats
                if ( item->GetParent() ) {
                    mRoot->maxStats.check( Max::SelfMs, Timer::ms( selfticks ) );
                    mRoot->maxStats.check( Max::Calls, item->mTimer.calls );
                    mRoot->maxStats.check( Max::Ms, Timer::ms( item->mTimer.ticks ) );
                    mRoot->maxStats.check( Max::Avg, item->mTimer.avgms() );
                    mRoot->maxStats.check( Max::SelfAvg, average( Timer::ms( selfticks ), item->mTimer.calls ) );
                }

                // compute child ticks for all children of children of this caller
                item->ForEachByRefNonEmpty( *this );
            }
            Caller &mTotals;
            Caller* mRoot;
        };

        /*
        Format a Caller's information. ComputeChildTicks will need to be used on the Root
        to generate mChildTicks for all Callers
        */
        struct Format {
            Format( const char *prefix ) : mPrefix(prefix) {}
            void operator()( Caller *item, bool islast ) const;

            const char *mPrefix;
        };

        struct FormatHtml {
            FormatHtml( Caller* root, FILE *f, Buffer<const char *> &prefix ) : mRoot(root), mFile(f), mPrefix(prefix) {}
            void operator()( Caller *item ) const;

            Caller* mRoot;
            FILE *mFile;
            Buffer<const char *> &mPrefix;
        };

        struct FormatHtmlTop {
            FormatHtmlTop( Caller* root, FILE *f ) : mRoot(root), mFile(f) {}
            void operator()( Caller *item, bool islast ) const;

            Caller* mRoot;
            FILE *mFile;
        };

        /* 
        Methods
        */

        // we're guaranteed to be null because of calloc. ONLY create Callers with "new"!
        Caller( const std::string& name, Caller *parent = NULL );

        ~Caller();

        void CopyToListNonEmpty( Buffer<Caller *> &list );

        Caller* Find( const std::string& name );
        Caller* Create(const std::string& name);

        template< class Mapto >
        void ForEachByRef( Mapto &mapto ) {
            for(Buckets::const_iterator iter = mBuckets.begin(); iter != mBuckets.end(); ++iter)
            {
                mapto(iter->second);
            }
        }

        template< class Mapto >
        void ForEachByRefNonEmpty( Mapto &mapto ) {
            for(Buckets::const_iterator iter = mBuckets.begin(); iter != mBuckets.end(); ++iter)
            {
                Caller* caller = iter->second;
                if(!caller->GetTimer().IsEmpty())
                {
                    mapto(caller);
                }
            }
        }

        template< class Mapto > 
        void ForEach( Mapto mapto ) {
            ForEachByRef( mapto );
        }

        template< class Mapto > 
        void ForEachNonEmpty( Mapto mapto ) {
            ForEachByRefNonEmpty( mapto );
        }

        Caller *GetParent();
        Timer &GetTimer();
        const std::string& GetName() const;
        bool IsActive() const;

        void Print( u32 indent = 0, bool islast = false );
        void PrintHtml( FILE *f, u32 indent = 0, bool islast = false );
        void SaveTopStats( u32 nitems, std::stringstream& stream );
        void PrintTopStats( u32 nitems );
        void Reset();
        void SetActive( bool active );
        void SetParent( Caller *parent );
        void SoftReset();
        void Start();
        void Stop();
        void *operator new ( size_t size );
        void operator delete ( void *p );

        Max maxStats;
        Buffer<char> mFormatter;
        Buffer<const char *> mHtmlFormatter;
        ColorRamp mColors;

    protected:
        std::string mName;
        Timer mTimer;
        u32 mNumChildren;
        Caller *mParent;
        Buckets mBuckets;

        bool mActive;
        u64 mChildTicks;
        
    };
    #pragma pack(pop)
}