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

#ifndef QHASH_H
#define QHASH_H

#include <QtCore/qatomic.h>
#include <QtCore/qchar.h>
#include <QtCore/qiterator.h>
#include <QtCore/qlist.h>

QT_MODULE(Core)

class QByteArray;
class QString;

inline uint qHash(char key) { return uint(key); }
inline uint qHash(uchar key) { return uint(key); }
inline uint qHash(signed char key) { return uint(key); }
inline uint qHash(ushort key) { return uint(key); }
inline uint qHash(short key) { return uint(key); }
inline uint qHash(uint key) { return key; }
inline uint qHash(int key) { return uint(key); }
inline uint qHash(ulong key)
{
    if (sizeof(ulong) > sizeof(uint)) {
        return uint((key >> (8 * sizeof(uint) - 1)) ^ key);
    } else {
        return uint(key);
    }
}
inline uint qHash(long key) { return qHash(ulong(key)); }
inline uint qHash(quint64 key)
{
    if (sizeof(quint64) > sizeof(uint)) {
        return uint((key >> (8 * sizeof(uint) - 1)) ^ key);
    } else {
        return uint(key);
    }
}
inline uint qHash(qint64 key) { return qHash(quint64(key)); }
inline uint qHash(QChar key) { return qHash(key.unicode()); }
Q_CORE_EXPORT uint qHash(const QByteArray &key);
Q_CORE_EXPORT uint qHash(const QString &key);

template <class T> inline uint qHash(const T *key)
{
    if (sizeof(const T *) > sizeof(uint))
        return qHash(reinterpret_cast<quint64>(key));
    else
        return uint(reinterpret_cast<ulong>(key));
}

struct Q_CORE_EXPORT QHashData
{
    struct Node {
        Node *next;
        uint h;
    };

    Node *fakeNext;
    Node **buckets;
    QBasicAtomic ref;
    int size;
    int nodeSize;
    short userNumBits;
    short numBits;
    int numBuckets;
    uint sharable : 1;

    void *allocateNode();
    void freeNode(void *node);
    QHashData *detach_helper(void (*node_duplicate)(Node *, void *), int nodeSize);
    void mightGrow();
    void hasShrunk();
    void rehash(int hint);
    void destroyAndFree();
    Node *firstNode();
    static Node *nextNode(Node *node);
    static Node *previousNode(Node *node);

    static QHashData shared_null;
};

inline void QHashData::mightGrow()
{
    if (size >= numBuckets)
        rehash(numBits + 1);
}

inline void QHashData::hasShrunk()
{
    if (size <= (numBuckets >> 3) && numBits > userNumBits)
        rehash(qMax(int(numBits) - 2, int(userNumBits)));
}

inline QHashData::Node *QHashData::firstNode()
{
    Node *e = reinterpret_cast<Node *>(this);
    Node **bucket = buckets;
    int n = numBuckets;
    while (n--) {
        if (*bucket != e)
            return *bucket;
        ++bucket;
    }
    return e;
}

struct QHashDummyValue
{
};

inline bool operator==(const QHashDummyValue & /* v1 */, const QHashDummyValue & /* v2 */)
{
    return true;
}

Q_DECLARE_TYPEINFO(QHashDummyValue, Q_MOVABLE_TYPE | Q_DUMMY_TYPE);

template <class Key, class T>
struct QHashDummyNode
{
    QHashDummyNode *next;
    uint h;
    Key key;

    inline QHashDummyNode(const Key &key0) : key(key0) {}
};

template <class Key, class T>
struct QHashNode
{
    QHashNode *next;
    uint h;
    Key key;
    T value;

    inline QHashNode(const Key &key0) : key(key0) {} // ### remove in 5.0
    inline QHashNode(const Key &key0, const T &value0) : key(key0), value(value0) {}
    inline bool same_key(uint h0, const Key &key0) { return h0 == h && key0 == key; }
};

#ifndef QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION
#define Q_HASH_DECLARE_INT_NODES(key_type) \
    template <class T> \
    struct QHashDummyNode<key_type, T> { \
        QHashDummyNode *next; \
        union { uint h; key_type key; }; \
\
        inline QHashDummyNode(key_type /* key0 */) {} \
    }; \
\
    template <class T> \
    struct QHashNode<key_type, T> { \
        QHashNode *next; \
        union { uint h; key_type key; }; \
        T value; \
\
        inline QHashNode(key_type /* key0 */) {} \
        inline QHashNode(key_type /* key0 */, const T &value0) : value(value0) {} \
        inline bool same_key(uint h0, key_type) { return h0 == h; } \
    }

#if defined(Q_BYTE_ORDER) && Q_BYTE_ORDER == Q_LITTLE_ENDIAN
Q_HASH_DECLARE_INT_NODES(short);
Q_HASH_DECLARE_INT_NODES(ushort);
#endif
Q_HASH_DECLARE_INT_NODES(int);
Q_HASH_DECLARE_INT_NODES(uint);
#undef Q_HASH_DECLARE_INT_NODES
#endif // QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION

template <class Key, class T>
class QHash
{
    typedef QHashDummyNode<Key, T> DummyNode;
    typedef QHashNode<Key, T> Node;

    union {
        QHashData *d;
        QHashNode<Key, T> *e;
    };

    static inline Node *concrete(QHashData::Node *node) {
        return reinterpret_cast<Node *>(node);
    }

public:
    inline QHash() : d(&QHashData::shared_null) { d->ref.ref(); }
    inline QHash(const QHash<Key, T> &other) : d(other.d) { d->ref.ref(); if (!d->sharable) detach(); }
    inline ~QHash() { if (!d->ref.deref()) freeData(d); }

    QHash<Key, T> &operator=(const QHash<Key, T> &other);

    bool operator==(const QHash<Key, T> &other) const;
    inline bool operator!=(const QHash<Key, T> &other) const { return !(*this == other); }

    inline int size() const { return d->size; }

    inline bool isEmpty() const { return d->size == 0; }

    inline int capacity() const { return d->numBuckets; }
    void reserve(int size);
    inline void squeeze() { reserve(1); }

    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const { return d->ref == 1; }
    inline void setSharable(bool sharable) { if (!sharable) detach(); d->sharable = sharable; }

    void clear();

    int remove(const Key &key);
    T take(const Key &key);

    bool contains(const Key &key) const;
    const Key key(const T &value) const;
    const T value(const Key &key) const;
    const T value(const Key &key, const T &defaultValue) const;
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;

    QList<Key> keys() const;
    QList<Key> keys(const T &value) const;
    QList<T> values() const;
    QList<T> values(const Key &key) const;
    int count(const Key &key) const;

    class const_iterator;

    class iterator
    {
        QHashData::Node *i;
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline operator Node *() const { return concrete(i); }
        inline iterator() : i(0) { }
        explicit inline iterator(void *node) : i(reinterpret_cast<QHashData::Node *>(node)) { }

        inline const Key &key() const { return concrete(i)->key; }
        inline T &value() const { return concrete(i)->value; }
        inline T &operator*() const { return concrete(i)->value; }
        inline T *operator->() const { return &concrete(i)->value; }
        inline bool operator==(const iterator &o) const { return i == o.i; }
        inline bool operator!=(const iterator &o) const { return i != o.i; }
        inline bool operator==(const const_iterator &o) const
            { return i == reinterpret_cast<const iterator &>(o).i; }
        inline bool operator!=(const const_iterator &o) const
            { return i != reinterpret_cast<const iterator &>(o).i; }

        inline iterator &operator++() {
            i = QHashData::nextNode(i);
            return *this;
        }
        inline iterator operator++(int) {
            iterator r = *this;
            i = QHashData::nextNode(i);
            return r;
        }
        inline iterator &operator--() {
            i = QHashData::previousNode(i);
            return *this;
        }
        inline iterator operator--(int) {
            iterator r = *this;
            i = QHashData::previousNode(i);
            return r;
        }
        inline iterator operator+(int j) const
        { iterator r = *this; if (j > 0) while (j--) ++r; else while (j++) --r; return r; }
        inline iterator operator-(int j) const { return operator+(-j); }
        inline iterator &operator+=(int j) { return *this = *this + j; }
        inline iterator &operator-=(int j) { return *this = *this - j; }
    };
    friend class iterator;

    class const_iterator
    {
        QHashData::Node *i;
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;
        typedef T value_type;
        typedef const T *pointer;
        typedef const T &reference;

        inline operator Node *() const { return concrete(i); }
        inline const_iterator() : i(0) { }
        explicit inline const_iterator(void *node)
            : i(reinterpret_cast<QHashData::Node *>(node)) { }
        inline const_iterator(const iterator &o)
        { i = reinterpret_cast<const const_iterator &>(o).i; }

        inline const Key &key() const { return concrete(i)->key; }
        inline const T &value() const { return concrete(i)->value; }
        inline const T &operator*() const { return concrete(i)->value; }
        inline const T *operator->() const { return &concrete(i)->value; }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }

        inline const_iterator &operator++() {
            i = QHashData::nextNode(i);
            return *this;
        }
        inline const_iterator operator++(int) {
            const_iterator r = *this;
            i = QHashData::nextNode(i);
            return r;
        }
        inline const_iterator &operator--() {
            i = QHashData::previousNode(i);
            return *this;
        }
        inline const_iterator operator--(int) {
            const_iterator r = *this;
            i = QHashData::previousNode(i);
            return r;
        }
        inline const_iterator operator+(int j) const
        { const_iterator r = *this; if (j > 0) while (j--) ++r; else while (j++) --r; return r; }
        inline const_iterator operator-(int j) const { return operator+(-j); }
        inline const_iterator &operator+=(int j) { return *this = *this + j; }
        inline const_iterator &operator-=(int j) { return *this = *this - j; }
    };
    friend class const_iterator;

    // STL style
    inline iterator begin() { detach(); return iterator(d->firstNode()); }
    inline const_iterator begin() const { return const_iterator(d->firstNode()); }
    inline const_iterator constBegin() const { return const_iterator(d->firstNode()); }
    inline iterator end() { detach(); return iterator(e); }
    inline const_iterator end() const { return const_iterator(e); }
    inline const_iterator constEnd() const { return const_iterator(e); }
    iterator erase(iterator it);

    // more Qt
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    inline int count() const { return d->size; }
    iterator find(const Key &key);
    const_iterator find(const Key &key) const;
    const_iterator constFind(const Key &key) const;
    iterator insert(const Key &key, const T &value);
    iterator insertMulti(const Key &key, const T &value);
    QHash<Key, T> &unite(const QHash<Key, T> &other);

    // STL compatibility
    inline bool empty() const { return isEmpty(); }

private:
    void detach_helper();
    void freeData(QHashData *d);
    Node **findNode(const Key &key, uint *hp = 0) const;
    Node *createNode(uint h, const Key &key, const T &value, Node **nextNode);
    void deleteNode(Node *node);

    static void duplicateNode(QHashData::Node *originalNode, void *newNode);
};

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::deleteNode(Node *node)
{
#ifdef Q_CC_BOR
    node->~QHashNode<Key, T>();
#elif defined(QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION)
    node->~QHashNode();
#else
    node->~Node();
#endif
    d->freeNode(node);
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::duplicateNode(QHashData::Node *node, void *newNode)
{
    Node *concreteNode = concrete(node);
    if (QTypeInfo<T>::isDummy) {
        (void) new (newNode) DummyNode(concreteNode->key);
    } else {
        (void) new (newNode) Node(concreteNode->key, concreteNode->value);
    }
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::Node *
QHash<Key, T>::createNode(uint ah, const Key &akey, const T &avalue, Node **anextNode)
{
    Node *node;

    if (QTypeInfo<T>::isDummy) {
        node = reinterpret_cast<Node *>(new (d->allocateNode()) DummyNode(akey));
    } else {
        node = new (d->allocateNode()) Node(akey, avalue);
    }

    node->h = ah;
    node->next = *anextNode;
    *anextNode = node;
    ++d->size;
    return node;
}

template <class Key, class T>
Q_INLINE_TEMPLATE QHash<Key, T> &QHash<Key, T>::unite(const QHash<Key, T> &other)
{
    QHash<Key, T> copy(other);
    const_iterator it = copy.constEnd();
    while (it != copy.constBegin()) {
        --it;
        insertMulti(it.key(), it.value());
    }
    return *this;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QHash<Key, T>::freeData(QHashData *x)
{
    Node *e_for_x = reinterpret_cast<Node *>(x);
    Node **bucket = reinterpret_cast<Node **>(x->buckets);
    int n = x->numBuckets;
    while (n--) {
        Node *cur = *bucket++;
        while (cur != e_for_x) {
            Node *next = cur->next;
            deleteNode(cur);
            cur = next;
        }
    }
    x->destroyAndFree();
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::clear()
{
    *this = QHash<Key,T>();
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE void QHash<Key, T>::detach_helper()
{
    size_t asize;
    if (QTypeInfo<T>::isDummy) {
        asize = reinterpret_cast<char *>(&reinterpret_cast<Node *>(&QHashData::shared_null)->value)
               - reinterpret_cast<char *>(&QHashData::shared_null);
    } else {
        asize = sizeof(Node);
    }

    QHashData *x = d->detach_helper(duplicateNode, int(asize));
    x = qAtomicSetPtr(&d, x);
    if (!x->ref.deref())
        freeData(x);
}

template <class Key, class T>
Q_INLINE_TEMPLATE QHash<Key, T> &QHash<Key, T>::operator=(const QHash<Key, T> &other)
{
    if (d != other.d) {
        QHashData *x = other.d;
        x->ref.ref();
        x = qAtomicSetPtr(&d, x);
        if (!x->ref.deref())
            freeData(x);
        if (!d->sharable)
            detach_helper();
    }
    return *this;
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QHash<Key, T>::value(const Key &akey) const
{
    Node *node = *findNode(akey);
    if (node == e) {
        return T();
    } else {
        return node->value;
    }
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QHash<Key, T>::value(const Key &akey, const T &adefaultValue) const
{
    Node *node = *findNode(akey);
    if (node == e) {
        return adefaultValue;
    } else {
        return node->value;
    }
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<Key> QHash<Key, T>::keys() const
{
    QList<Key> res;
    const_iterator i = begin();
    while (i != end()) {
        res.append(i.key());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<Key> QHash<Key, T>::keys(const T &avalue) const
{
    QList<Key> res;
    const_iterator i = begin();
    while (i != end()) {
        if (i.value() == avalue)
            res.append(i.key());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE const Key QHash<Key, T>::key(const T &avalue) const
{
    const_iterator i = begin();
    while (i != end()) {
        if (i.value() == avalue)
            return i.key();
        ++i;
    }

    return Key();
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QHash<Key, T>::values() const
{
    QList<T> res;
    const_iterator i = begin();
    while (i != end()) {
        res.append(i.value());
        ++i;
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QList<T> QHash<Key, T>::values(const Key &akey) const
{
    QList<T> res;
    Node *node = *findNode(akey);
    if (node != e) {
        do {
            res.append(node->value);
        } while ((node = node->next) != e && node->key == akey);
    }
    return res;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE int QHash<Key, T>::count(const Key &akey) const
{
    int cnt = 0;
    Node *node = *findNode(akey);
    if (node != e) {
        do {
            ++cnt;
        } while ((node = node->next) != e && node->key == akey);
    }
    return cnt;
}

template <class Key, class T>
Q_INLINE_TEMPLATE const T QHash<Key, T>::operator[](const Key &akey) const
{
    return value(akey);
}

template <class Key, class T>
Q_INLINE_TEMPLATE T &QHash<Key, T>::operator[](const Key &akey)
{
    detach();
    d->mightGrow();

    uint h;
    Node **node = findNode(akey, &h);
    if (*node == e)
        return createNode(h, akey, T(), node)->value;
    return (*node)->value;
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::insert(const Key &akey,
                                                                         const T &avalue)
{
    detach();
    d->mightGrow();

    uint h;
    Node **node = findNode(akey, &h);
    if (*node == e)
        return iterator(createNode(h, akey, avalue, node));

    if (!QTypeInfo<T>::isDummy)
        (*node)->value = avalue;
    return iterator(*node);
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::insertMulti(const Key &akey,
                                                                              const T &avalue)
{
    detach();
    d->mightGrow();

    uint h;
    Node **nextNode = findNode(akey, &h);
    return iterator(createNode(h, akey, avalue, nextNode));
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE int QHash<Key, T>::remove(const Key &akey)
{
    detach();

    int oldSize = d->size;
    Node **node = findNode(akey);
    if (*node != e) {
        bool deleteNext = true;
        do {
            Node *next = (*node)->next;
            deleteNext = (next != e && next->key == (*node)->key);
            deleteNode(*node);
            *node = next;
            --d->size;
        } while (deleteNext);
        d->hasShrunk();
    }
    return oldSize - d->size;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE T QHash<Key, T>::take(const Key &akey)
{
    detach();

    Node **node = findNode(akey);
    if (*node != e) {
        T t = (*node)->value;
        Node *next = (*node)->next;
        deleteNode(*node);
        *node = next;
        --d->size;
        d->hasShrunk();
        return t;
    }
    return T();
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::erase(iterator it)
{
    if (it == iterator(e))
        return it;

    iterator ret = it;
    ++ret;

    Node *node = it;
    Node **node_ptr = reinterpret_cast<Node **>(&d->buckets[node->h % d->numBuckets]);
    while (*node_ptr != node)
        node_ptr = &(*node_ptr)->next;
    *node_ptr = node->next;
    deleteNode(node);
    --d->size;
    return ret;
}

template <class Key, class T>
Q_INLINE_TEMPLATE void QHash<Key, T>::reserve(int asize)
{
    detach();
    d->rehash(-qMax(asize, 1));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::const_iterator QHash<Key, T>::find(const Key &akey) const
{
    return const_iterator(*findNode(akey));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::const_iterator QHash<Key, T>::constFind(const Key &akey) const
{
    return const_iterator(*findNode(akey));
}

template <class Key, class T>
Q_INLINE_TEMPLATE typename QHash<Key, T>::iterator QHash<Key, T>::find(const Key &akey)
{
    detach();
    return iterator(*findNode(akey));
}

template <class Key, class T>
Q_INLINE_TEMPLATE bool QHash<Key, T>::contains(const Key &akey) const
{
    return *findNode(akey) != e;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE typename QHash<Key, T>::Node **QHash<Key, T>::findNode(const Key &akey,
                                                                            uint *ahp) const
{
    Node **node;
    uint h = qHash(akey);

    if (d->numBuckets) {
        node = reinterpret_cast<Node **>(&d->buckets[h % d->numBuckets]);
        while (*node != e && !(*node)->same_key(h, akey))
            node = &(*node)->next;
    } else {
        node = const_cast<Node **>(reinterpret_cast<const Node * const *>(&e));
    }
    if (ahp)
        *ahp = h;
    return node;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE bool QHash<Key, T>::operator==(const QHash<Key, T> &other) const
{
    if (size() != other.size())
        return false;
    if (d == other.d)
        return true;

    const_iterator it1 = begin();
    const_iterator it2 = other.begin();

    while (it1 != end()) {
        if (!(it1.key() == it2.key()))
            return false;
        if (!QTypeInfo<T>::isDummy && !(it1.value() == it2.value()))
            return false;
        ++it2;
        ++it1;
    }
    return true;
}

template <class Key, class T>
class QMultiHash : public QHash<Key, T>
{
public:
    QMultiHash() {}
    QMultiHash(const QHash<Key, T> &other) : QHash<Key, T>(other) {}

    inline typename QHash<Key, T>::iterator replace(const Key &key, const T &value);
    inline typename QHash<Key, T>::iterator insert(const Key &key, const T &value);

    inline QMultiHash &operator+=(const QMultiHash &other)
    { unite(other); return *this; }
    inline QMultiHash operator+(const QMultiHash &other) const
    { QMultiHash result = *this; result += other; return result; }

private:
    T &operator[](const Key &key);
    const T operator[](const Key &key) const;
};

template <class Key, class T>
Q_INLINE_TEMPLATE Q_TYPENAME QHash<Key, T>::iterator QMultiHash<Key, T>::replace(const Key &akey, const T &avalue)
{ return QHash<Key, T>::insert(akey, avalue); }

template <class Key, class T>
Q_INLINE_TEMPLATE Q_TYPENAME QHash<Key, T>::iterator QMultiHash<Key, T>::insert(const Key &akey, const T &avalue)
{ return QHash<Key, T>::insertMulti(akey, avalue); }


Q_DECLARE_ASSOCIATIVE_ITERATOR(Hash)
Q_DECLARE_MUTABLE_ASSOCIATIVE_ITERATOR(Hash)

#endif // QHASH_H
