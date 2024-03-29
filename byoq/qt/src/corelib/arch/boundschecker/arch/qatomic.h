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

#ifndef BOUNDSCHECKER_QATOMIC_H
#define BOUNDSCHECKER_QATOMIC_H

#include <QtCore/qglobal.h>

__forceinline int q_atomic_xchg(int newval)
{
    static int lockvar = 0;
    int *pointer = &lockvar;

    __asm {
        mov EDX,pointer
        mov ECX,newval
        xchg dword ptr[EDX],ECX
        mov newval,ECX
    }
    return newval;
}

inline void q_atomic_lock()
{
    while (q_atomic_xchg(~0) != 0)
        ;
}

inline void q_atomic_unlock()
{
    q_atomic_xchg(0);
}

inline int q_atomic_test_and_set_int(volatile int *pointer, int expected, int newval)
{
    q_atomic_lock();
    if (*pointer == expected) {
        *pointer = newval;
        q_atomic_unlock();
        return 1;
    }
    q_atomic_unlock();
    return 0;
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_ptr(volatile void *pointer, void *expected, void *newval)
{
    q_atomic_lock();
    if (*reinterpret_cast<void * volatile *>(pointer) == expected) {
        *reinterpret_cast<void * volatile *>(pointer) = newval;
        q_atomic_unlock();
        return 1;
    }
    q_atomic_unlock();
    return 0;
}

inline int q_atomic_increment(volatile int *pointer)
{
    q_atomic_lock();
    int ret = ++(*pointer);
    q_atomic_unlock();
    return ret;
}

inline int q_atomic_decrement(volatile int *pointer)
{
    q_atomic_lock();
    int ret = --(*pointer);
    q_atomic_unlock();
    return ret;
}

inline int q_atomic_set_int(volatile int *pointer, int newval)
{
    q_atomic_lock();
    int ret = *pointer;
    *pointer = newval;
    q_atomic_unlock();
    return ret;
}

inline void *q_atomic_set_ptr(volatile void *pointer, void *newval)
{
    q_atomic_lock();
    void *ret = *reinterpret_cast<void * volatile *>(pointer);
    *reinterpret_cast<void * volatile *>(pointer) = newval;
    q_atomic_unlock();
    return ret;

}

#endif // BOUNDSCHECKER_QATOMIC_H
