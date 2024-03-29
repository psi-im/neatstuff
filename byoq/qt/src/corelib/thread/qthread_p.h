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

#ifndef QTHREAD_P_H
#define QTHREAD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QAbstractItemModel*.  This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//
//

#include "qplatformdefs.h"
#include "QtCore/qthread.h"
#include "QtCore/qmutex.h"
#include "QtCore/qstack.h"
#include "QtCore/qwaitcondition.h"
#include "private/qobject_p.h"

class QAbstractEventDispatcher;
class QEventLoop;

class QPostEvent
{
public:
    QObject *receiver;
    QEvent *event;
    inline QPostEvent()
        : receiver(0), event(0)
    { }
    inline QPostEvent(QObject *r, QEvent *e)
        : receiver(r), event(e)
    { }
};

class QPostEventList : public QList<QPostEvent>
{
public:
    int recursion;
    QMutex mutex;

    inline QPostEventList()
        : QList<QPostEvent>(), recursion(0)
    { }
};

class Q_CORE_EXPORT QThreadData
{
public:
    QThreadData();
    ~QThreadData();

    static QThreadData *get(QThread *thread);
    
    int id;
    bool quitNow;
    QAbstractEventDispatcher *eventDispatcher;
    QStack<QEventLoop *> eventLoops;
    QPostEventList postEventList;
    bool canWait;
    void **tls;
};

#ifndef QT_NO_THREAD
class QThreadPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QThread)

public:
    QThreadPrivate();

    mutable QMutex mutex;

    bool running;
    bool finished;
    bool terminated;

    uint stackSize;
    QThread::Priority priority;

    static void setCurrentThread(QThread *thread);

    static QThread *threadForId(int id);

#ifdef Q_OS_UNIX
    pthread_t thread_id;
    QWaitCondition thread_done;

    static void *start(void *arg);
    static void finish(void *arg);
#endif

#ifdef Q_OS_WIN32
    HANDLE handle;
    unsigned int id;
    int waiters;
    bool terminationEnabled, terminatePending;

    static unsigned int __stdcall start(void *);
    static void finish(void *, bool lockAnyway=true);
#endif // Q_OS_WIN32

    QThreadData data;
};

#else // QT_NO_THREAD

class QThreadPrivate : public QObjectPrivate
{
public:
    QThreadPrivate(QThread *threadInstance)
    { Q_ASSERT(!QThread::instance); QThread::instance = threadInstance; }
    QThreadData data;
    static void setCurrentThread(QThread*) {}
    static QThread *threadForId(int) { return QThread::currentThread(); }

    Q_DECLARE_PUBLIC(QThread)
};

#endif // QT_NO_THREAD

#endif // QTHREAD_P_H
