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

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#include <QtCore/qstring.h>
#include <QtNetwork/qabstractsocket.h>

QT_MODULE(Network)

struct sockaddr;
class QHostAddressPrivate;

class Q_NETWORK_EXPORT QIPv6Address
{
public:
    inline quint8 &operator [](int index) { return c[index]; }
    inline quint8 operator [](int index) const { return c[index]; }
    quint8 c[16];
};

typedef QIPv6Address Q_IPV6ADDR;

class Q_NETWORK_EXPORT QHostAddress
{
public:
    enum SpecialAddress {
        Null,
        Broadcast,
        LocalHost,
        LocalHostIPv6,
        Any,
        AnyIPv6
    };

    QHostAddress();
    explicit QHostAddress(quint32 ip4Addr);
    explicit QHostAddress(quint8 *ip6Addr);
    explicit QHostAddress(const Q_IPV6ADDR &ip6Addr);
    explicit QHostAddress(const sockaddr *sockaddr);
    explicit QHostAddress(const QString &address);
    QHostAddress(const QHostAddress &copy);
    QHostAddress(SpecialAddress address);
    ~QHostAddress();

    QHostAddress &operator=(const QHostAddress &other);
    QHostAddress &operator=(const QString &address);

    void setAddress(quint32 ip4Addr);
    void setAddress(quint8 *ip6Addr);
    void setAddress(const Q_IPV6ADDR &ip6Addr);
    void setAddress(const sockaddr *sockaddr);
    bool setAddress(const QString &address);

    QAbstractSocket::NetworkLayerProtocol protocol() const;
    quint32 toIPv4Address() const;
    Q_IPV6ADDR toIPv6Address() const;

    QString toString() const;

    QString scopeId() const;
    void setScopeId(const QString &id);

    bool operator ==(const QHostAddress &address) const;
    bool operator ==(SpecialAddress address) const;
    bool isNull() const;
    void clear();

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT quint32 ip4Addr() const { return toIPv4Address(); }
    inline QT3_SUPPORT bool isIPv4Address() const { return protocol() == QAbstractSocket::IPv4Protocol
                                                      || protocol() == QAbstractSocket::UnknownNetworkLayerProtocol; }
    inline QT3_SUPPORT bool isIp4Addr() const  { return protocol() == QAbstractSocket::IPv4Protocol
                                                      || protocol() == QAbstractSocket::UnknownNetworkLayerProtocol; }
    inline QT3_SUPPORT bool isIPv6Address() const { return protocol() == QAbstractSocket::IPv6Protocol; }
#endif

private:
    QHostAddressPrivate *d;
};

inline bool operator ==(QHostAddress::SpecialAddress address1, const QHostAddress &address2)
{ return address2 == address1; }

#ifndef QT_NO_DEBUG_STREAM
Q_NETWORK_EXPORT QDebug operator<<(QDebug, const QHostAddress &);
#endif

#endif // QHOSTADDRESS_H
