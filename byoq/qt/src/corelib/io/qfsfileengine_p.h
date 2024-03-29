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

#ifndef QFSFILEENGINE_P_H
#define QFSFILEENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qplatformdefs.h"
#include "QtCore/qfsfileengine.h"
#include "private/qabstractfileengine_p.h"

class QFSFileEnginePrivate : public QAbstractFileEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileEngine)

public:
#ifdef Q_WS_WIN
    static QString fixToQtSlashes(const QString &path);
    static QByteArray win95Name(const QString &path);
    static QString longFileName(const QString &path);
#else
    static inline QString fixToQtSlashes(const QString &path) { return path; }
#endif

    QString file;

    int fd;
    mutable uint sequential : 1;
    QByteArray ungetchBuffer;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
#ifdef Q_OS_UNIX
    mutable uint isSymLink : 1;
#endif
#ifdef Q_WS_WIN
    mutable DWORD fileAttrib;
#else
    mutable QT_STATBUF st;
#endif
    bool doStat() const;
    int sysOpen(const QString &, int flags);

    FILE *fh;
    bool closeFileHandle;
    enum LastIOCommand
    {
        IOFlushCommand,
        IOReadCommand,
        IOWriteCommand
    };
    LastIOCommand  lastIOCommand;
    
protected:
    QFSFileEnginePrivate();

    void init();

#if defined(Q_OS_WIN32)
    QAbstractFileEngine::FileFlags getPermissions() const;
    QString getLink() const;
#endif
};

#endif // QFSFILEENGINE_P_H
