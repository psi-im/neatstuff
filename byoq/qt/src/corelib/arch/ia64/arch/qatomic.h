/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef IA64_QATOMIC_H
#define IA64_QATOMIC_H

#include <QtCore/qglobal.h>

#if defined(Q_CC_INTEL)

// intrinsics provided by the Intel C++ Compiler
#include <ia64intrin.h>

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{ return static_cast<int>(_InterlockedCompareExchange(ptr, newval, expected)) == expected; }

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return static_cast<int>(_InterlockedCompareExchange_acq(reinterpret_cast<volatile uint *>(ptr),
                                                          newval, expected)) == expected; }

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return static_cast<int>(_InterlockedCompareExchange_rel(reinterpret_cast<volatile uint *>(ptr),
                                                          newval, expected)) == expected; }

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    return _InterlockedCompareExchangePointer(reinterpret_cast<void * volatile*>(ptr),
                                              newval, expected) == expected;
}

inline int q_atomic_increment(volatile int *ptr)
{ return _InterlockedIncrement(ptr); }

inline int q_atomic_decrement(volatile int *ptr)
{ return _InterlockedDecrement(ptr); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{ return _InterlockedExchange(ptr, newval); }

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    return _InterlockedExchangePointer(reinterpret_cast<void * volatile *>(ptr), newval);
}

#else // !Q_CC_INTEL

#  if defined(Q_CC_GNU)

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    int ret;
    asm volatile("mov ar.ccv=%2\n"
                 ";;\n"
                 "cmpxchg4.acq %0=%1,%3,ar.ccv\n"
                 : "=r" (ret), "+m" (*ptr)
                 : "r" (expected), "r" (newval)
                 : "memory");
    return ret == expected;
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    int ret;
    asm volatile("mov ar.ccv=%2\n"
                 ";;\n"
                 "cmpxchg4.rel %0=%1,%3,ar.ccv\n"
                 : "=r" (ret), "+m" (*ptr)
                 : "r" (expected), "r" (newval)
                 : "memory");
    return ret == expected;
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval) {
    void *ret;
    asm volatile("mov ar.ccv=%2\n"
                 ";;\n"
                 "cmpxchg8.acq %0=%1,%3,ar.ccv\n"
                 : "=r" (ret), "+m" (*reinterpret_cast<volatile unsigned long *>(ptr))
                 : "r" (expected), "r" (newval)
                 : "memory");
    return ret == expected;
}

#  else // !Q_CC_GNU

extern "C" {
    Q_CORE_EXPORT int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
    Q_CORE_EXPORT int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
} // extern "C"

#  endif // Q_CC_GNU

inline int q_atomic_increment(volatile int * const ptr)
{
    register int expected;
    for (;;) {
        expected = *ptr;
        if (q_atomic_test_and_set_int(ptr, expected, expected + 1)) break;
    }
    return expected != -1;
}

inline int q_atomic_decrement(volatile int * const ptr)
{
    register int expected;
    for (;;) {
        expected = *ptr;
        if (q_atomic_test_and_set_int(ptr, expected, expected - 1)) break;
    }
    return expected != 1;
}

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    register int expected;
    for (;;) {
        expected = *ptr;
        if (q_atomic_test_and_set_int(ptr, expected, newval)) break;
    }
    return expected;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    register void *expected;
    for (;;) {
        expected = *reinterpret_cast<void * volatile *>(ptr);
        if (q_atomic_test_and_set_ptr(ptr, expected, newval)) break;
    }
    return expected;
}

#endif // Q_CC_INTEL

#endif // IA64_QATOMIC_H
