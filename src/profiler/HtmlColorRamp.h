#pragma once

#include "profiler/Platform.h"
#include "profiler/Buffer.h"

namespace profiling
{
/*
	===================
	ColorRamp for HTML
	===================
	*/

	struct ColorF {
		ColorF() {}
		ColorF( f32 r_, f32 g_, f32 b_ ) : r(r_), g(g_), b(b_) {}
		f32 r, g, b;
	};

	struct ColorRamp {
		struct Marker {
			Marker() {}
			Marker( const ColorF &color_, f32 value_ ) : color(color_), value(value_) {}
			ColorF color;
			f32 value;
		};

		ColorRamp() {}

		void clear() {
			mColors.Clear();
		}
		
		const char *value( f32 pos ) const {
			ColorF base(0, 0, 0);
			u32 pre = 0, post = 0;
			for ( pre = 0; pre < mColors.Size() - 1; pre++ )
				if ( mColors[pre+1].value >= pos )
					break;
			post = pre + 1;
			if ( ( pre < mColors.Size() ) && ( post < mColors.Size() ) ) {
				const Marker &a = mColors[pre], &b = mColors[post];
				f32 dist = ( b.value - a.value ), posw = ( pos - a.value ), bw = ( posw / dist ), aw = 1 - bw;
				base = ColorF( a.color.r * aw + b.color.r * bw, a.color.g * aw + b.color.g * bw, a.color.b * aw + b.color.b * bw );
			}
			u8 r = u8(base.r * 255.0f), g = u8(base.g * 255.0f), b = u8(base.b * 255.0f);
			static threadlocal char buffer[8][32], bufferon = 0;
			sprintf_s( buffer[bufferon&7], "#%02x%02x%02x", r, g, b );
			return buffer[bufferon++&7];
		}

		ColorRamp &push( const ColorF &color, f32 value ) { mColors.Push( Marker( color, value ) ); return *this; }

		Buffer<Marker> mColors;
	};

}