/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the QtNetwork module of the Qt Toolkit.
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

#ifndef QURLINFO_H
#define QURLINFO_H

#include <QtCore/qdatetime.h>
#include <QtCore/qstring.h>
#include <QtCore/qiodevice.h>

QT_MODULE(Network)

#ifndef QT_NO_URLINFO

class QUrl;
class QUrlInfoPrivate;

class Q_NETWORK_EXPORT QUrlInfo
{
public:
    enum PermissionSpec {
        ReadOwner = 00400, WriteOwner = 00200, ExeOwner = 00100,
        ReadGroup = 00040, WriteGroup = 00020, ExeGroup = 00010,
        ReadOther = 00004, WriteOther = 00002, ExeOther = 00001 };

    QUrlInfo();
    QUrlInfo(const QUrlInfo &ui);
    QUrlInfo(const QString &name, int permissions, const QString &owner,
             const QString &group, qint64 size, const QDateTime &lastModified,
             const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
             bool isWritable, bool isReadable, bool isExecutable);
    QUrlInfo(const QUrl &url, int permissions, const QString &owner,
             const QString &group, qint64 size, const QDateTime &lastModified,
             const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
             bool isWritable, bool isReadable, bool isExecutable);
    QUrlInfo &operator=(const QUrlInfo &ui);
    virtual ~QUrlInfo();

    virtual void setName(const QString &name);
    virtual void setDir(bool b);
    virtual void setFile(bool b);
    virtual void setSymLink(bool b);
    virtual void setOwner(const QString &s);
    virtual void setGroup(const QString &s);
    virtual void setSize(qint64 size);
    virtual void setWritable(bool b);
    virtual void setReadable(bool b);
    virtual void setPermissions(int p);
    virtual void setLastModified(const QDateTime &dt);

    bool isValid() const;

    QString name() const;
    int permissions() const;
    QString owner() const;
    QString group() const;
    qint64 size() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;
    bool isDir() const;
    bool isFile() const;
    bool isSymLink() const;
    bool isWritable() const;
    bool isReadable() const;
    bool isExecutable() const;

    static bool greaterThan(const QUrlInfo &i1, const QUrlInfo &i2,
                             int sortBy);
    static bool lessThan(const QUrlInfo &i1, const QUrlInfo &i2,
                          int sortBy);
    static bool equal(const QUrlInfo &i1, const QUrlInfo &i2,
                       int sortBy);

    bool operator==(const QUrlInfo &i) const;

private:
    QUrlInfoPrivate *d;
};

#endif // QT_NO_URLINFO

#endif // QURLINFO_H
