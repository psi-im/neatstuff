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

#include "qplatformdefs.h"
#include "qmutex.h"

#ifndef QT_NO_THREAD
#include "qatomic.h"
#include "qmutex_p.h"
#include <errno.h>
#include <string.h>


static void report_error(int code, const char *where, const char *what)
{
    if (code != 0)
        qWarning("%s: %s failure: %s", where, what, strerror(code));
}


QMutexPrivate::QMutexPrivate(QMutex::RecursionMode mode)
    : lock(0), owner(0), count(0), recursive(mode == QMutex::Recursive), wakeup(false)
{
    report_error(pthread_mutex_init(&mutex, NULL), "QMutex", "mutex init");
    report_error(pthread_cond_init(&cond, NULL), "QMutex", "cv init");
}

QMutexPrivate::~QMutexPrivate()
{
    report_error(pthread_cond_destroy(&cond), "QMutex", "cv destroy");
    report_error(pthread_mutex_destroy(&mutex), "QMutex", "mutex destroy");
}

ulong QMutexPrivate::self()
{ return (ulong) pthread_self(); }

void QMutexPrivate::wait()
{
    report_error(pthread_mutex_lock(&mutex), "QMutex::lock()", "mutex lock");
    while (!wakeup)
        report_error(pthread_cond_wait(&cond, &mutex), "QMutex::lock()", "cv wait");
    wakeup = false;
    report_error(pthread_mutex_unlock(&mutex), "QMutex::lock()", "mutex unlock");
}

void QMutexPrivate::wakeUp()
{
    report_error(pthread_mutex_lock(&mutex), "QMutex::unlock()", "mutex lock");
    wakeup = true;
    report_error(pthread_cond_signal(&cond), "QMutex::unlock()", "cv signal");
    report_error(pthread_mutex_unlock(&mutex), "QMutex::unlock()", "mutex unlock");
}

#endif // QT_NO_THREAD
