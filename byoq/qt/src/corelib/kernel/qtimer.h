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

#ifndef QTIMER_H
#define QTIMER_H

#ifndef QT_NO_QOBJECT

#include <QtCore/qbasictimer.h> // conceptual inheritance
#include <QtCore/qobject.h>

QT_MODULE(Core)

class Q_CORE_EXPORT QTimer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool singleShot READ isSingleShot WRITE setSingleShot)
    Q_PROPERTY(int interval READ interval WRITE setInterval)
public:
    explicit QTimer(QObject *parent = 0);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QTimer(QObject *parent, const char *name);
#endif
    ~QTimer();

    inline bool isActive() const { return id >= 0; }
    int timerId() const { return id; }

    void setInterval(int msec);
    int interval() const { return inter; }

    inline void setSingleShot(bool singleShot);
    inline bool isSingleShot() const { return single; }

    static void singleShot(int msec, QObject *receiver, const char *member);

public Q_SLOTS:
    void start(int msec);

    void start();
    void stop();

#ifdef QT3_SUPPORT
    inline QT_MOC_COMPAT void changeInterval(int msec) { start(msec); };
    QT_MOC_COMPAT int start(int msec, bool sshot);
#endif

Q_SIGNALS:
    void timeout();

protected:
    void timerEvent(QTimerEvent *);

private:
    Q_DISABLE_COPY(QTimer)

    inline int startTimer(int){ return -1;}
    inline void killTimer(int){}

    int id, inter, del;
    uint single : 1;
    uint nulltimer : 1;
};

inline void QTimer::setSingleShot(bool asingleShot) { single = asingleShot; }

#endif // QT_NO_QOBJECT

#endif // QTIMER_H
