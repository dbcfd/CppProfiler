#pragma once

#include "profiler/Platform.h"

#undef min
#undef max

namespace profiling
{

	inline u32 nextpow2( u32 x ) {
		x |= ( x >>  1 );
		x |= ( x >>  2 );
		x |= ( x >>  4 );
		x |= ( x >>  8 );
		x |= ( x >> 16 );
		return ( x + 1 );
	}

	template< class type >
	void zeroarray( type *array, size_t count ) {
		memset( array, 0, count * sizeof( type ) );
	}

	template< class type >
	type *makepointer( type *base, size_t byteoffset ) {
		return (type *)((const char *)base + byteoffset);
	}

	template< class type >
	void swapitems( type &a, type &b ) {
		type tmp = a;
		a = b;
		b = tmp;
	}

	

	template< class type >
	const type &min( const type &a, const type &b ) {
		return ( a < b ) ? a : b;
	}

	template< class type >
	const type &max( const type &a, const type &b ) {
		return ( a < b ) ? b : a;
	}
	
	template< class type1, class type2 >
	f64 average( type1 sum, type2 count ) {
		return ( count ) ? f64(sum)/f64(count) : 0;
	}
}