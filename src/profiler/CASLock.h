#pragma once

#include <windows.h>

#include "profiler/Platform.h"

#define YIELD() Sleep(0);

namespace profiling {
    template< class type >
    inline bool CAS( volatile type &ptr_, const type old_, const type new_ ) {
        __asm {
            mov eax, [old_]
            mov edx, [new_]
            mov ecx, [ptr_]
            lock cmpxchg dword ptr [ecx], edx
                sete al
        }
    }

    struct CASLock {
        CASLock() : mLock(0)
        {

        }
        CASLock(u32 lock) : mLock(lock)
        {

        }
        void Acquire() { while ( !CAS( mLock, u32(0), u32(1) ) ) YIELD() }
        void Release() { while ( !CAS( mLock, u32(1), u32(0) ) ) YIELD() }
        bool TryAcquire() { return CAS( mLock, u32(0), u32(1) ); }
        bool TryRelease() { return CAS( mLock, u32(1), u32(0) ); }
        u32 Value() const { return mLock; }
        //protected:
        volatile u32 mLock;
    };
}