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

#ifndef QOBJECT_H
#define QOBJECT_H

#ifndef QT_NO_QOBJECT

#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#ifdef QT_INCLUDE_COMPAT
#include <QtCore/qcoreevent.h>
#endif

QT_MODULE(Core)

class QEvent;
class QTimerEvent;
class QChildEvent;
struct QMetaObject;
class QVariant;
class QObjectPrivate;
class QObject;
class QThread;
class QWidget;
#ifndef QT_NO_REGEXP
class QRegExp;
#endif
#ifndef QT_NO_USERDATA
class QObjectUserData;
#endif

typedef QList<QObject*> QObjectList;

#if defined Q_CC_MSVC && _MSC_VER < 1300
template<typename T> inline T qFindChild(const QObject *o, const QString &name = QString(), T = 0);
template<typename T> inline QList<T> qFindChildren(const QObject *o, const QString &name = QString(), T = 0);
# ifndef QT_NO_REGEXP
template<typename T> inline QList<T> qFindChildren(const QObject *o, const QRegExp &re, T = 0);
# endif
#else
template<typename T> inline T qFindChild(const QObject *, const QString & = QString());
template<typename T> inline QList<T> qFindChildren(const QObject *, const QString & = QString());
# ifndef QT_NO_REGEXP
template<typename T> inline QList<T> qFindChildren(const QObject *, const QRegExp &);
# endif
#endif

class QObjectData {
public:
    virtual ~QObjectData() = 0;
    QObject *q_ptr;
    QObject *parent;
    QObjectList children;

    uint isWidget : 1;
    uint pendTimer : 1;
    uint blockSig : 1;
    uint wasDeleted : 1;
    uint ownObjectName : 1;
    uint sendChildEvents : 1;
    uint receiveChildEvents : 1;
    uint unused : 25;
    int postedEvents;
#ifdef QT3_SUPPORT
    int postedChildInsertedEvents;
#else
    int reserved;
#endif
};


class Q_CORE_EXPORT QObject
{
    Q_OBJECT
    Q_PROPERTY(QString objectName READ objectName WRITE setObjectName)
    Q_DECLARE_PRIVATE(QObject)

public:
    explicit QObject(QObject *parent=0);
    virtual ~QObject();

    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

#ifdef qdoc
    static QString tr(const char *, const char *);
    static QString trUtf8(const char *, const char *);
    virtual const QMetaObject *metaObject() const;
#endif
#ifdef QT_NO_TRANSLATION
    static QString tr(const char *sourceText, const char * = 0)
        { return QString::fromLatin1(sourceText); }
#ifndef QT_NO_TEXTCODEC
    static QString trUtf8(const char *sourceText, const char * = 0)
        { return QString::fromUtf8(sourceText); }
#endif
#endif //QT_NO_TRANSLATION

    QString objectName() const;
    void setObjectName(const QString &name);

    inline bool isWidgetType() const { return d_ptr->isWidget; }

    inline bool signalsBlocked() const { return d_ptr->blockSig; }
    bool blockSignals(bool b);

    QThread *thread() const;
    void moveToThread(QThread *thread);

    int startTimer(int interval);
    void killTimer(int id);

#ifndef QT_NO_MEMBER_TEMPLATES
    template<typename T>
    inline T findChild(const QString &name = QString()) const
    { return qFindChild<T>(this, name); }

    template<typename T>
    inline QList<T> findChildren(const QString &name = QString()) const
    { return qFindChildren<T>(this, name); }

#ifndef QT_NO_REGEXP
    template<typename T>
    inline QList<T> findChildren(const QRegExp &re) const
    { return qFindChildren<T>(this, re); }
#endif
#endif

#ifdef QT3_SUPPORT
    QT3_SUPPORT QObject *child(const char *objName, const char *inheritsClass = 0,
                   bool recursiveSearch = true) const;
    QT3_SUPPORT QObjectList queryList(const char *inheritsClass = 0,
                          const char *objName = 0,
                          bool regexpMatch = true,
                          bool recursiveSearch = true) const;
#endif
    inline const QObjectList &children() const { return d_ptr->children; }

    void setParent(QObject *);
    void installEventFilter(QObject *);
    void removeEventFilter(QObject *);


    static bool connect(const QObject *sender, const char *signal,
                        const QObject *receiver, const char *member, Qt::ConnectionType =
#ifdef QT3_SUPPORT
                        Qt::AutoCompatConnection
#else
                        Qt::AutoConnection
#endif
        );
    inline bool connect(const QObject *sender, const char *signal,
                        const char *member, Qt::ConnectionType type =
#ifdef QT3_SUPPORT
                        Qt::AutoCompatConnection
#else
                        Qt::AutoConnection
#endif
        ) const;

    static bool disconnect(const QObject *sender, const char *signal,
                           const QObject *receiver, const char *member);
    inline bool disconnect(const char *signal = 0,
                           const QObject *receiver = 0, const char *member = 0)
        { return disconnect(this, signal, receiver, member); }
    inline bool disconnect(const QObject *receiver, const char *member = 0)
        { return disconnect(this, 0, receiver, member); }

    void dumpObjectTree();
    void dumpObjectInfo();

#ifndef QT_NO_PROPERTIES
    bool setProperty(const char *name, const QVariant &value);
    QVariant property(const char *name) const;
#endif // QT_NO_PROPERTIES

#ifndef QT_NO_USERDATA
    static uint registerUserData();
    void setUserData(uint id, QObjectUserData* data);
    QObjectUserData* userData(uint id) const;
#endif // QT_NO_USERDATA

Q_SIGNALS:
    void destroyed(QObject * = 0);

public:
    inline QObject *parent() const { return d_ptr->parent; }

    inline bool inherits(const char *classname) const
        { return const_cast<QObject *>(this)->qt_metacast(classname) != 0; }

public Q_SLOTS:
    void deleteLater();

protected:
    QObject *sender() const;
    int receivers(const char* signal) const;

    virtual void timerEvent(QTimerEvent *);
    virtual void childEvent(QChildEvent *);
    virtual void customEvent(QEvent *);

    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QObject(QObject *parent, const char *name);
    inline QT3_SUPPORT void insertChild(QObject *o)
        { if (o) o->setParent(this); }
    inline QT3_SUPPORT void removeChild(QObject *o)
        { if (o) o->setParent(0); }
    inline QT3_SUPPORT bool isA(const char *classname) const
        { return qstrcmp(classname, metaObject()->className()) == 0; }
    inline QT3_SUPPORT const char *className() const { return metaObject()->className(); }
    inline QT3_SUPPORT const char *name() const { return objectName().latin1_helper(); }
    inline QT3_SUPPORT const char *name(const char *defaultName) const
        { QString s = objectName(); return s.isEmpty()?defaultName:s.latin1_helper(); }
    inline QT3_SUPPORT void setName(const char *name) { setObjectName(QLatin1String(name)); }
protected:
    inline QT3_SUPPORT bool checkConnectArgs(const char *signal,
                                  const QObject *,
                                  const char *member)
        { return QMetaObject::checkConnectArgs(signal, member); }
    static inline QT3_SUPPORT QByteArray normalizeSignalSlot(const char *signalSlot)
        { return QMetaObject::normalizedSignature(signalSlot); }
#endif

protected:
    QObject(QObjectPrivate &dd, QObject *parent = 0);

protected:
    QObjectData *d_ptr;

    static const QMetaObject staticQtMetaObject;

    friend struct QMetaObject;
    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QCoreApplication;
    friend class QCoreApplicationPrivate;
    friend class QWidget;
    friend class QThreadData;

private:
    Q_DISABLE_COPY(QObject)
    Q_PRIVATE_SLOT(d_func(), void reregisterTimers(void *))
};

inline bool QObject::connect(const QObject *asender, const char *asignal,
                             const char *amember, Qt::ConnectionType atype) const
{ return connect(asender, asignal, this, amember, atype); }

#ifndef QT_NO_USERDATA
class Q_CORE_EXPORT QObjectUserData {
public:
    virtual ~QObjectUserData();
};
#endif


Q_CORE_EXPORT void qt_qFindChildren_helper(const QObject *parent, const QString &name, const QRegExp *re,
                         const QMetaObject &mo, QList<void*> *list);
Q_CORE_EXPORT QObject *qt_qFindChild_helper(const QObject *parent, const QString &name, const QMetaObject &mo);


#if defined Q_CC_MSVC && _MSC_VER < 1300

template<typename T>
inline T qFindChild(const QObject *o, const QString &name, T)
{ return static_cast<T>(qt_qFindChild_helper(o, name, ((T)0)->staticMetaObject)); }

template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QString &name, T)
{
    QList<T> list;
    qt_qFindChildren_helper(o, name, 0, ((T)0)->staticMetaObject,
                        reinterpret_cast<QList<void *>*>(&list));
    return list;
}

template<typename T>
inline T qFindChild(const QObject *o, const QString &name)
{ return qFindChild<T>(o, name, T(0)); }

template<typename T>
inline T qFindChild(const QObject *o)
{ return qFindChild<T>(o, QString(), T(0)); }

template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QString &name)
{ return qFindChildren<T>(o, name, T(0)); }

template<typename T>
inline QList<T> qFindChildren(const QObject *o)
{ return qFindChildren<T>(o, QString(), T(0)); }

#ifndef QT_NO_REGEXP
template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QRegExp &re, T)
{
    QList<T> list;
    qt_qFindChildren_helper(o, 0, &re, ((T)0)->staticMetaObject,
                        reinterpret_cast<QList<void*>*>(&list));
    return list;
}

template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QRegExp &re)
{ return qFindChildren<T>(o, re, T(0)); }

#endif

template <class T> inline T qobject_cast_helper(QObject *object, T)
{ return static_cast<T>(((T)0)->staticMetaObject.cast(object)); }

template <class T> inline T qobject_cast_helper(const QObject *object, T)
{ return static_cast<T>(const_cast<const QObject *>(((T)0)->staticMetaObject.cast(const_cast<QObject *>(object)))); }

template <class T>
inline T qobject_cast(QObject *object)
{ return qobject_cast_helper<T>(object, T(0)); }

template <class T>
inline T qobject_cast(const QObject *object)
{ return qobject_cast_helper<T>(object, T(0)); }

#define Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qobject_cast_helper<IFace *>(QObject *object, IFace *) \
{ return (IFace *)(object ? object->qt_metacast(IId) : 0); } \
template <> inline IFace *qobject_cast_helper<IFace *>(const QObject *object, IFace *) \
{ return (IFace *)(object ? const_cast<QObject *>(object)->qt_metacast(IId) : 0); }

#else

template<typename T>
inline T qFindChild(const QObject *o, const QString &name)
{ return static_cast<T>(qt_qFindChild_helper(o, name, reinterpret_cast<T>(0)->staticMetaObject)); }

template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QString &name)
{
    QList<T> list;
    qt_qFindChildren_helper(o, name, 0, reinterpret_cast<T>(0)->staticMetaObject,
                         reinterpret_cast<QList<void *>*>(&list));
    return list;
}

#ifndef QT_NO_REGEXP
template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QRegExp &re)
{
    QList<T> list;
    qt_qFindChildren_helper(o, 0, &re, reinterpret_cast<T>(0)->staticMetaObject,
                        reinterpret_cast<QList<void*>*>(&list));
    return list;
}
#endif

template <class T>
inline T qobject_cast(QObject *object)
{ return static_cast<T>(reinterpret_cast<T>(0)->staticMetaObject.cast(object)); }

template <class T>
inline T qobject_cast(const QObject *object)
{ return static_cast<T>(const_cast<const QObject *>(reinterpret_cast<T>(0)->staticMetaObject.cast(const_cast<QObject *>(object)))); }


#define Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qobject_cast<IFace *>(QObject *object) \
{ return reinterpret_cast<IFace *>((object ? object->qt_metacast(IId) : 0)); } \
template <> inline IFace *qobject_cast<IFace *>(const QObject *object) \
{ return reinterpret_cast<IFace *>((object ? const_cast<QObject *>(object)->qt_metacast(IId) : 0)); }

#endif

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QObject *);
#endif

#endif

#endif // QOBJECT_H
