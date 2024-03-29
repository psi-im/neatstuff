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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#ifdef Q_WS_WIN
# include "QtCore/qt_windows.h"
#endif
#include "QtCore/qlibrary.h"
#include "QtCore/qpointer.h"
#include "QtCore/qstringlist.h"
#include "QtCore/qplugin.h"

#ifndef QT_NO_LIBRARY

class QLibraryPrivate
{
public:

#ifdef Q_WS_WIN
    HINSTANCE
#else
    void *
#endif
    pHnd;

    QString fileName, qualifiedFileName;
    int majorVerNum;

    bool load();
    bool loadPlugin(); // loads and resolves instance
    bool unload();
    void release();
    void *resolve(const char *);

    static QLibraryPrivate *findOrCreate(const QString &fileName, int verNum = -1);

    QtPluginInstanceFunction instance;
    uint qt_version;
    QString lastModified;

    bool isPlugin();


private:
    explicit QLibraryPrivate(const QString &canonicalFileName, int verNum = -1);
    ~QLibraryPrivate();

    bool load_sys();
    bool unload_sys();
    void *resolve_sys(const char *);

    QAtomic libraryRefCount;
    QAtomic libraryUnloadCount;

    enum {IsAPlugin, IsNotAPlugin, MightBeAPlugin } pluginState;
    friend class QLibraryPrivateHasFriends;
};

#endif // QT_NO_LIBRARY

#endif // QLIBRARY_P_H
