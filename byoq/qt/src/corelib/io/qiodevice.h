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

#ifndef QIODEVICE_H
#define QIODEVICE_H

#ifndef QT_NO_QOBJECT
#include <QtCore/qobject.h>
#else
#include <QtCore/qobjectdefs.h>
#endif
#include <QtCore/qstring.h>

#ifdef open
#error qiodevice.h must be included before any header file that defines open
#endif

QT_MODULE(Core)

class QByteArray;
class QIODevicePrivate;

class Q_CORE_EXPORT QIODevice
#ifndef QT_NO_QOBJECT
    : public QObject
#endif
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif
public:
    enum OpenModeFlag {
        NotOpen = 0x0000,
        ReadOnly = 0x0001,
        WriteOnly = 0x0002,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 0x0004,
        Truncate = 0x0008,
        Text = 0x0010,
        Unbuffered = 0x0020
    };
    Q_DECLARE_FLAGS(OpenMode, OpenModeFlag)

    QIODevice();
#ifndef QT_NO_QOBJECT
    explicit QIODevice(QObject *parent);
#endif
    virtual ~QIODevice();

    OpenMode openMode() const;

    void setTextModeEnabled(bool enabled);
    bool isTextModeEnabled() const;

    bool isOpen() const;
    bool isReadable() const;
    bool isWritable() const;
    virtual bool isSequential() const;

    virtual bool open(OpenMode mode);
    virtual void close();

    virtual qint64 pos() const;
    virtual qint64 size() const;
    virtual bool seek(qint64 pos);
    virtual bool atEnd() const;
    virtual bool reset();

    virtual qint64 bytesAvailable() const;
    virtual qint64 bytesToWrite() const;

    qint64 read(char *data, qint64 maxlen);
    QByteArray read(qint64 maxlen);
    QByteArray readAll();
    qint64 readLine(char *data, qint64 maxlen);
    QByteArray readLine(qint64 maxlen = 0);
    virtual bool canReadLine() const;

    qint64 write(const char *data, qint64 len);
    inline qint64 write(const QByteArray &data)
    { return write(data.constData(), data.size()); }

    qint64 peek(char *data, qint64 maxlen);
    QByteArray peek(qint64 maxlen);

    virtual bool waitForReadyRead(int msecs);
    virtual bool waitForBytesWritten(int msecs);

    void ungetChar(char c);
    inline bool putChar(char c) { return write(&c, 1) == 1; }
    inline bool getChar(char *c)
    { char ch; bool result = read(&ch, 1) == 1; if (c) *c = ch; return result; }

    QString errorString() const;

#ifndef QT_NO_QOBJECT
Q_SIGNALS:
    void readyRead();
    void bytesWritten(qint64 bytes);
    void aboutToClose();
#endif

protected:
#ifdef QT_NO_QOBJECT
    QIODevice(QIODevicePrivate &dd);
#else
    QIODevice(QIODevicePrivate &dd, QObject *parent = 0);
#endif
    virtual qint64 readData(char *data, qint64 maxlen) = 0;
    virtual qint64 readLineData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len) = 0;

    void setOpenMode(OpenMode openMode);

    void setErrorString(const QString &errorString);

#ifdef QT_NO_QOBJECT
    QIODevicePrivate *d_ptr;
#endif

private:
    Q_DECLARE_PRIVATE(QIODevice)
    Q_DISABLE_COPY(QIODevice)

#ifdef QT3_SUPPORT
public:
    typedef qint64 Offset;

    inline QT3_SUPPORT int flags() const { return (int) openMode(); }
    inline QT3_SUPPORT int mode() const { return (int) openMode(); }
    inline QT3_SUPPORT int state() const;

    inline QT3_SUPPORT bool isDirectAccess() const { return !isSequential(); }
    inline QT3_SUPPORT bool isSequentialAccess() const { return isSequential(); }
    inline QT3_SUPPORT bool isCombinedAccess() const { return false; }
    inline QT3_SUPPORT bool isBuffered() const { return true; }
    inline QT3_SUPPORT bool isRaw() const { return false; }
    inline QT3_SUPPORT bool isSynchronous() const { return true; }
    inline QT3_SUPPORT bool isAsynchronous() const { return false; }
    inline QT3_SUPPORT bool isTranslated() const { return (openMode() & Text) != 0; }
    inline QT3_SUPPORT bool isInactive() const { return !isOpen(); }

    typedef int Status;
    QT3_SUPPORT Status status() const;
    QT3_SUPPORT void resetStatus();

    inline QT3_SUPPORT Offset at() const { return pos(); }
    inline QT3_SUPPORT bool at(Offset offset) { return seek(offset); }

    inline QT3_SUPPORT qint64 readBlock(char *data, quint64 maxlen) { return read(data, maxlen); }
    inline QT3_SUPPORT qint64 writeBlock(const char *data, quint64 len) { return write(data, len); }
    inline QT3_SUPPORT qint64 writeBlock(const QByteArray &data) { return write(data); }

    inline QT3_SUPPORT int getch() { char c; return getChar(&c) ? int(c) : -1; }
    inline QT3_SUPPORT int putch(int c) { return putChar(c) ? int(c) : -1; }
    inline QT3_SUPPORT int ungetch(int c) { ungetChar(c); return c; }
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QIODevice::OpenMode)

#ifdef QT3_SUPPORT
static QT3_SUPPORT_VARIABLE const uint IO_Direct = 0x0100;
static QT3_SUPPORT_VARIABLE const uint IO_Sequential = 0x0200;
static QT3_SUPPORT_VARIABLE const uint IO_Combined = 0x0300;
static QT3_SUPPORT_VARIABLE const uint IO_TypeMask = 0x0300;

static QT3_SUPPORT_VARIABLE const uint IO_Raw = 0x0000;
static QT3_SUPPORT_VARIABLE const uint IO_Async = 0x0000;

#define IO_ReadOnly QIODevice::ReadOnly
#define IO_WriteOnly QIODevice::WriteOnly
#define IO_ReadWrite QIODevice::ReadWrite
#define IO_Append QIODevice::Append
#define IO_Truncate QIODevice::Truncate
#define IO_Translate QIODevice::Text
#define IO_ModeMask 0x00ff

static QT3_SUPPORT_VARIABLE const uint IO_Open = 0x1000;
static QT3_SUPPORT_VARIABLE const uint IO_StateMask = 0xf000;

static QT3_SUPPORT_VARIABLE const uint IO_Ok = 0;
static QT3_SUPPORT_VARIABLE const uint IO_ReadError = 1;
static QT3_SUPPORT_VARIABLE const uint IO_WriteError = 2;
static QT3_SUPPORT_VARIABLE const uint IO_FatalError = 3;
static QT3_SUPPORT_VARIABLE const uint IO_ResourceError = 4;
static QT3_SUPPORT_VARIABLE const uint IO_OpenError = 5;
static QT3_SUPPORT_VARIABLE const uint IO_ConnectError = 5;
static QT3_SUPPORT_VARIABLE const uint IO_AbortError = 6;
static QT3_SUPPORT_VARIABLE const uint IO_TimeOutError = 7;
static QT3_SUPPORT_VARIABLE const uint IO_UnspecifiedError	= 8;

inline QT3_SUPPORT int QIODevice::state() const
{
    return isOpen() ? 0x1000 : 0;
}
#endif

#if !defined(QT_NO_DEBUG_STREAM)
class QDebug;
Q_CORE_EXPORT QDebug operator<<(QDebug debug, QIODevice::OpenMode modes);
#endif

#endif // QIODEVICE_H
