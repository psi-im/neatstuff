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

#ifndef QFSFILEENGINE_H
#define QFSFILEENGINE_H

#include <QtCore/qabstractfileengine.h>

QT_MODULE(Core)

class QFSFileEnginePrivate;

class Q_CORE_EXPORT QFSFileEngine : public QAbstractFileEngine
{
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    QFSFileEngine();
    explicit QFSFileEngine(const QString &file);
    ~QFSFileEngine();

    bool open(QIODevice::OpenMode openMode);
    bool open(QIODevice::OpenMode flags, FILE *fh);
    bool close();
    bool flush();
    qint64 size() const;
    qint64 pos() const;
    bool seek(qint64);
    bool isSequential() const;
    bool remove();
    bool copy(const QString &newName);
    bool rename(const QString &newName);
    bool link(const QString &newName);
    bool mkdir(const QString &dirName, bool createParentDirectories) const;
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const;
    bool setSize(qint64 size);
    bool caseSensitive() const;
    bool isRelativePath() const;
    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;
    FileFlags fileFlags(FileFlags type) const;
    bool setPermissions(uint perms);
    QString fileName(QAbstractFileEngine::FileName file) const;
    uint ownerId(QAbstractFileEngine::FileOwner) const;
    QString owner(FileOwner) const;
    QDateTime fileTime(FileTime time) const;
    void setFileName(const QString &file);
    int handle() const;

    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames);
    Iterator *endEntryList();
    
    qint64 read(char *data, qint64 maxlen);
    qint64 readLine(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);

    bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0);
    bool supportsExtension(Extension extension) const;

    //FS only!!
    bool open(QIODevice::OpenMode flags, int fd);
    static bool setCurrentPath(const QString &path);
    static QString currentPath(const QString &path = QString());
    static QString homePath();
    static QString rootPath();
    static QString tempPath();
    static QFileInfoList drives();

protected:
    QFSFileEngine(QFSFileEnginePrivate &dd);
};

#endif // QFSFILEENGINE_H
