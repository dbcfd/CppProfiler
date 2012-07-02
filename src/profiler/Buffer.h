#pragma once

#include "profiler/Platform.h"

namespace profiling
{

    /*
    =============
    Buffer - Don't use for anything with a constructor/destructor. Doesn't shrink on popping
    =============
    */

    template< class type >
    struct Buffer {
        Buffer() : mBuffer(NULL), mAlloc(0), mItems(0) { Resize( 4 ); }
        Buffer( u32 size ) : mBuffer(NULL), mAlloc(0), mItems(0) { Resize( size ); }
        ~Buffer() { free( mBuffer ); }

        void Clear() { mItems = ( 0 ); }
        type *Data() { return ( mBuffer ); }
        void EnsureCapacity( u32 capacity ) { if ( capacity >= mAlloc ) Resize( capacity * 2 ); }
        type *Last() { return ( &mBuffer[ mItems - 1 ] ); }
        void Push( const type &item ) { EnsureCapacity( mItems + 1 ); mBuffer[ mItems++ ] = ( item ); }
        type &Pop() { return ( mBuffer[ --mItems ] ); }

        void Resize( u32 newsize ) {
            mAlloc = nextpow2( newsize );
            mBuffer = (type *)realloc( mBuffer, mAlloc * sizeof( type ) );
        }

        u32 Size() const { return mItems; }

        template< class Compare >
        void Sort( Compare comp ) {
            if ( mItems <= 1 )
                return;

            Buffer scratch( mItems );

            // merge sort with scratch buffer
            type *src = Data(), *dst = scratch.Data();
            for( u32 log = 2; log < mItems * 2; log *= 2 ) {
                type *out = dst;
                for( u32 i = 0; i < mItems; i += log ) {
                    u32 lo = i, lo2 = min( i + log / 2, mItems );
                    u32 hi = lo2, hi2 = min( lo + log, mItems );
                    while ( ( lo < lo2 ) && ( hi < hi2 ) )
                        *out++ = ( comp( src[lo], src[hi] ) ) ? src[lo++] : src[hi++];
                    while ( lo < lo2 ) *out++ = src[lo++];
                    while ( hi < hi2 ) *out++ = src[hi++];
                }

                swapitems( src, dst );
            }

            if ( src != mBuffer )
                swapitems( mBuffer, scratch.mBuffer );
        }

        template< class Mapto >
        void ForEachByRef( Mapto &mapto, u32 limit ) {
            limit = ( limit < mItems ) ? limit : mItems;
            u32 last = limit - 1;
            for ( u32 i = 0; i < limit; ++i )
                mapto( mBuffer[ i ], i == last );
        }

        template< class Mapto >	void ForEach( Mapto mapto, u32 limit ) { ForEachByRef( mapto, limit ); }
        template< class Mapto >	void ForEach( Mapto mapto ) { ForEachByRef( mapto, mItems ); }

        type &operator[] ( u32 index ) { return ( mBuffer[ index ] ); }
        const type &operator[] ( u32 index ) const { return ( mBuffer[ index ] ); }

    protected:
        type *mBuffer;
        u32 mAlloc, mItems;
    };	

}