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

#ifndef QUUID_H
#define QUUID_H

#include <QtCore/qstring.h>

QT_MODULE(Core)

#if defined(Q_OS_WIN32)
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    ulong   Data1;
    ushort  Data2;
    ushort  Data3;
    uchar   Data4[8];
} GUID, *REFGUID, *LPGUID;
#endif
#endif

struct Q_CORE_EXPORT QUuid
{
    enum Variant {
        VarUnknown        =-1,
        NCS                = 0, // 0 - -
        DCE                = 2, // 1 0 -
        Microsoft        = 6, // 1 1 0
        Reserved        = 7  // 1 1 1
    };

    enum Version {
        VerUnknown        =-1,
        Time                = 1, // 0 0 0 1
        EmbeddedPOSIX        = 2, // 0 0 1 0
        Name                = 3, // 0 0 1 1
        Random                = 4  // 0 1 0 0
    };

    QUuid()
    {
        data1 = 0;
        data2 = 0;
        data3 = 0;
        for(int i = 0; i < 8; i++)
            data4[i] = 0;
    }
    QUuid(uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3, uchar b4, uchar b5, uchar b6, uchar b7, uchar b8)
    {
        data1 = l;
        data2 = w1;
        data3 = w2;
        data4[0] = b1;
        data4[1] = b2;
        data4[2] = b3;
        data4[3] = b4;
        data4[4] = b5;
        data4[5] = b6;
        data4[6] = b7;
        data4[7] = b8;
    }
#ifndef QT_NO_QUUID_STRING
    QUuid(const QString &);
    QUuid(const char *);
    QString toString() const;
    operator QString() const { return toString(); }
#endif
    bool isNull() const;

    bool operator==(const QUuid &orig) const
    {
        uint i;
        if (data1 != orig.data1 || data2 != orig.data2 ||
             data3 != orig.data3)
            return false;

        for(i = 0; i < 8; i++)
            if (data4[i] != orig.data4[i])
                return false;

        return true;
    }

    bool operator!=(const QUuid &orig) const
    {
        return !(*this == orig);
    }

    bool operator<(const QUuid &other) const;
    bool operator>(const QUuid &other) const;

#if defined(Q_OS_WIN32)
    // On Windows we have a type GUID that is used by the platform API, so we
    // provide convenience operators to cast from and to this type.
    QUuid(const GUID &guid)
    {
        data1 = guid.Data1;
        data2 = guid.Data2;
        data3 = guid.Data3;
        for(int i = 0; i < 8; i++)
            data4[i] = guid.Data4[i];
    }

    QUuid &operator=(const GUID &guid)
    {
        *this = QUuid(guid);
        return *this;
    }

    operator GUID() const
    {
        GUID guid = { data1, data2, data3, { data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7] } };
        return guid;
    }

    bool operator==(const GUID &guid) const
    {
        return *this == QUuid(guid);
    }

    bool operator!=(const GUID &guid) const
    {
        return !(*this == guid);
    }
#endif
    static QUuid createUuid();
    QUuid::Variant variant() const;
    QUuid::Version version() const;

    uint    data1;
    ushort  data2;
    ushort  data3;
    uchar   data4[8];
};

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QUuid &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QUuid &);
#endif

#endif // QUUID_H
