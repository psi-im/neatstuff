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

#include "qvector.h"
#include "qtools_p.h"
#include <string.h>

QVectorData QVectorData::shared_null = { Q_ATOMIC_INIT(1), 0, 0, true };

QVectorData* QVectorData::malloc(int sizeofTypedData, int size, int sizeofT, QVectorData* init)
{
    QVectorData* p = (QVectorData *)qMalloc(sizeofTypedData + (size - 1) * sizeofT);
    ::memcpy(p, init, sizeofTypedData + (qMin(size, init->alloc) - 1) * sizeofT);
    return p;
}

int QVectorData::grow(int sizeofTypedData, int size, int sizeofT, bool excessive)
{
    if (excessive)
        return size + size / 2;
    return qAllocMore(size * sizeofT, sizeofTypedData - sizeofT) / sizeofT;
}

/*! 
    \class QVector
    \brief The QVector class is a template class that provides a dynamic array.

    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    QVector\<T\> is one of Qt's generic \l{container classes}. It
    stores its items in adjacent memory locations and provides fast
    index-based access.

    QList\<T\>, QLinkedList\<T\>, and QVarLengthArray\<T\> provide
    similar functionality. Here's an overview:

    \list
    \i For most purposes, QList is the right class to use. Operations
       like prepend() and insert() are usually faster than with
       QVector because of the way QList stores its items in memory,
       and its index-based API is more convenient than QLinkedList's
       iterator-based API. It also expands to less code in your
       executable.
    \i If you need a real linked list, with guarantees of \l{constant
       time} insertions in the middle of the list and iterators to
       items rather than indexes, use QLinkedList.
    \i If you want the items to occupy adjacent memory positions,
       use QVector.
    \i If you want a low-level variable-size array, QVarLengthArray
    may be sufficient.
    \endlist

    Here's an example of a QVector that stores integers and a QVector
    that stores QString values:

    \code
        QVector<int> integerVector;
        QVector<QString> stringVector;
    \endcode

    QVector stores a vector (or array) of items. Typically, vectors
    are created with an initial size. For example, the following code
    constructs a QVector with 200 elements:

    \code
        QVector<QString> vector(200);
    \endcode

    The elements are automatically initialized with a
    \l{default-constructed value}. If you want to initialize the
    vector with a different value, pass that value as the second
    argument to the constructor:

    \code
        QVector<QString> vector(200, "Pass");
    \endcode

    You can also call fill() at any time to fill the vector with a
    value.

    QVector uses 0-based indexes, just like C++ arrays. To access the
    item at a particular index position, you can use operator[](). On
    non-const vectors, operator[]() returns a reference to the item
    that can be used on the left side of an assignment:

    \code
        if (vector[0] == "Liz")
            vector[0] = "Elizabeth";
    \endcode

    For read-only access, an alternative syntax is to use at():

    \code
        for (int i = 0; i < vector.size(); ++i) {
            if (vector.at(i) == "Alfonso")
                cout << "Found Alfonso at position " << i << endl;
        }
    \endcode

    at() can be faster than operator[](), because it never causes a
    \l{deep copy} to occur.

    Another way to access the data stored in a QVector is to call
    data(). The function returns a pointer to the first item in the
    vector. You can use the pointer to directly access and modify the
    elements stored in the vector. The pointer is also useful if you
    need to pass a QVector to a function that accepts a plain C++
    array.

    If you want to find all occurrences of a particular value in a
    vector, use indexOf() or lastIndexOf(). The former searches
    forward starting from a given index position, the latter searches
    backward. Both return the index of the matching item if they found
    one; otherwise, they return -1. For example:

    \code
        int i = vector.indexOf("Harumi");
        if (i != -1)
            cout << "First occurrence of Harumi is at position " << i << endl;
    \endcode

    If you simply want to check whether a vector contains a
    particular value, use contains(). If you want to find out how
    many times a particular value occurs in the vector, use count().

    QVector provides these basic functions to add, move, and remove
    items: insert(), replace(), remove(), prepend(), append(). With
    the exception of append(), these functions can be slow (\l{linear
    time}) for large vectors, because they require moving many items
    in the vector by one position in memory. If you want a container
    class that provides fast insertion/removal in the middle, use
    QList or QLinkedList instead.

    Unlike plain C++ arrays, QVectors can be resized at any time by
    calling resize(). If the new size is larger than the old size,
    QVector might need to reallocate the whole vector. QVector tries
    to reduce the number of reallocations by preallocating up to twice
    as much memory as the actual data needs.

    If you know in advance approximately how many items the QVector
    will contain, you can call reserve(), asking QVector to
    preallocate a certain amount of memory. You can also call
    capacity() to find out how much memory QVector actually
    allocated.

    QVector's value type must be an \l{assignable data type}. This
    covers most data types that are commonly used, but the compiler
    won't let you, for example, store a QWidget as a value; instead,
    store a QWidget *. A few functions have additional requirements;
    for example, indexOf() and lastIndexOf() expect the value type to
    support \c operator==(). These requirements are documented on a
    per-function basis.

    Like the other container classes, QVector provides \l{Java-style
    iterators} (QVectorIterator and QMutableVectorIterator) and
    \l{STL-style iterators} (QVector::const_iterator and
    QVector::iterator). In practice, these are rarely used, because
    you can use indexes into the QVector.

    In addition to QVector, Qt also provides QVarLengthArray, a very
    low-level class with little functionality that is optimized for
    speed.

    QVector does \e not support inserting, prepending, appending or replacing
    with references to its own values. Doing so will cause your application to
    abort with an error message.

    \sa QVectorIterator, QMutableVectorIterator, QList, QLinkedList
*/

/*!
    \fn QVector<T> QVector::mid(int pos, int length = -1) const

    Returns a vector whose elements are copied from this vector,
    starting at position \a pos. If \a length is -1 (the default), all
    elements after \a pos are copied; otherwise \a length elements (or
    all remaining elements if there are less than \a length elements)
    are copied.
*/


/*! \fn QVector::QVector()

    Constructs an empty vector.

    \sa resize()
*/

/*! \fn QVector::QVector(int size)

    Constructs a vector with an initial size of \a size elements.

    The elements are initialized with a \l{default-constructed
    value}.

    \sa resize()
*/

/*! \fn QVector::QVector(int size, const T &value)

    Constructs a vector with an initial size of \a size elements.
    Each element is initialized with \a value.

    \sa resize(), fill()
*/

/*! \fn QVector::QVector(const QVector &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QVector is
    \l{implicitly shared}. This makes returning a QVector from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and that takes \l{linear time}.

    \sa operator=()
*/

/*! \fn QVector::~QVector()

    Destroys the vector.
*/

/*! \fn QVector &QVector::operator=(const QVector &other)

    Assigns \a other to this vector and returns a reference to this
    vector.
*/

/*! \fn bool QVector::operator==(const QVector &other) const

    Returns true if \a other is equal to this vector; otherwise
    returns false.

    Two vectors are considered equal if they contain the same values
    in the same order.

    This function requires the value type to have an implementation
    of \c operator==().

    \sa operator!=()
*/

/*! \fn bool QVector::operator!=(const QVector &other) const

    Returns true if \a other is not equal to this vector; otherwise
    returns false.

    Two vectors are considered equal if they contain the same values
    in the same order.

    This function requires the value type to have an implementation
    of \c operator==().

    \sa operator==()
*/

/*! \fn int QVector::size() const

    Returns the number of items in the vector.

    \sa isEmpty(), resize()
*/

/*! \fn bool QVector::isEmpty() const

    Returns true if the vector has size 0; otherwise returns false.

    \sa size(), resize()
*/

/*! \fn void QVector::resize(int size)

    Sets the size of the vector to \a size. If \a size is greater than the
    current size, elements are added to the end; the new elements are
    initialized with a \l{default-constructed value}. If \a size is less
    than the current size, elements are removed from the end.

    \sa size()
*/

/*! \fn int QVector::capacity() const

    Returns the maximum number of items that can be stored in the
    vector without forcing a reallocation.

    The sole purpose of this function is to provide a means of fine
    tuning QVector's memory usage. In general, you will rarely ever
    need to call this function. If you want to know how many items are
    in the vector, call size().

    \sa reserve(), squeeze()
*/

/*! \fn void QVector::reserve(int size)

    Attempts to allocate memory for at least \a size elements. If you
    know in advance how large the vector will be, you can call this
    function, and if you call resize() often you are likely to get
    better performance. If \a size is an underestimate, the worst
    that will happen is that the QVector will be a bit slower.

    The sole purpose of this function is to provide a means of fine
    tuning QVector's memory usage. In general, you will rarely ever
    need to call this function. If you want to change the size of the
    vector, call resize().

    \sa squeeze(), capacity()
*/

/*! \fn void QVector::squeeze()

    Releases any memory not required to store the items.

    The sole purpose of this function is to provide a means of fine
    tuning QVector's memory usage. In general, you will rarely ever
    need to call this function.

    \sa reserve(), capacity()
*/

/*! \fn void QVector::detach()

    \internal
*/

/*! \fn bool QVector::isDetached() const

    \internal
*/

/*! \fn void QVector::setSharable(bool sharable)

    \internal
*/

/*! \fn T *QVector::data()

    Returns a pointer to the data stored in the vector. The pointer
    can be used to access and modify the items in the vector.

    Example:
    \code
        QVector<int> vector(10);
        int *data = vector.data();
        for (int i = 0; i < 10; ++i)
            data[i] = 2 * i;
    \endcode

    The pointer remains valid as long as the vector isn't
    reallocated.

    This function is mostly useful to pass a vector to a function
    that accepts a plain C++ array.

    \sa constData(), operator[]()
*/

/*! \fn const T *QVector::data() const

    \overload
*/

/*! \fn const T *QVector::constData() const

    Returns a const pointer to the data stored in the vector. The
    pointer can be used to access the items in the vector.
    The pointer remains valid as long as the vector isn't
    reallocated.

    This function is mostly useful to pass a vector to a function
    that accepts a plain C++ array.

    \sa data(), operator[]()
*/

/*! \fn void QVector::clear()

    Removes all the elements from the vector.

    Same as resize(0).
*/

/*! \fn const T &QVector::at(int i) const

    Returns the item at index position \a i in the vector.

    \a i must be a valid index position in the vector (i.e., 0 <= \a
    i < size()).

    \sa value(), operator[]()
*/

/*! \fn T &QVector::operator[](int i)

    Returns the item at index position \a i as a modifiable reference.

    \a i must be a valid index position in the vector (i.e., 0 <= \a i
    < size()).

    \sa at(), value()
*/

/*! \fn const T &QVector::operator[](int i) const

    \overload

    Same as at(\a i).
*/

/*! \fn void QVector::append(const T &value)

    Inserts \a value at the end of the vector.

    Example:
    \code
        QVector<QString> vector(0);
        vector.append("one");
        vector.append("two");
        vector.append("three");
        // vector: ["one", "two", three"]
    \endcode

    This is the same as calling resize(size() + 1) and assigning \a
    value to the new last element in the vector.

    This operation is relatively fast, because QVector typically
    allocates more memory than necessary, so it can grow without
    reallocating the entire vector each time.

    \sa operator<<(), prepend(), insert()
*/

/*! \fn void QVector::prepend(const T &value)

    Inserts \a value at the beginning of the vector.

    Example:
    \code
        QVector<QString> vector;
        vector.prepend("one");
        vector.prepend("two");
        vector.prepend("three");
        // vector: ["three", "two", "one"]
    \endcode

    This is the same as vector.insert(0, \a value).

    For large vectors, this operation can be slow (\l{linear time}),
    because it requires moving all the items in the vector by one
    position further in memory. If you want a container class that
    provides a fast prepend() function, use QList or QLinkedList
    instead.

    \sa append(), insert()
*/

/*! \fn void QVector::insert(int i, const T &value)

    Inserts \a value at index position \a i in the vector. If \a i is
    0, the value is prepended to the vector. If \a i is size(), the
    value is appended to the vector.

    Example:
    \code
        QVector<QString> vector;
        vector << "alpha" << "beta" << "delta";
        vector.insert(2, "gamma");
        // vector: ["alpha", "beta", "gamma", "delta"]
    \endcode

    For large vectors, this operation can be slow (\l{linear time}),
    because it requires moving all the items at indexes \a i and
    above by one position further in memory. If you want a container
    class that provides a fast insert() function, use QLinkedList
    instead.

    \sa append(), prepend(), remove()
*/

/*! \fn void QVector::insert(int i, int count, const T &value)

    \overload

    Inserts \a count copies of \a value at index position \a i in the
    vector.

    Example:
    \code
        QVector<double> vector;
        vector << 2.718 << 1.442 << 0.4342;
        vector.insert(1, 3, 9.9);
        // vector: [2.718, 9.9, 9.9, 9.9, 1.442, 0.4342]
    \endcode
*/

/*! \fn QVector::iterator QVector::insert(iterator before, const T &value)

    \overload

    Inserts \a value in front of the item pointed to by the iterator
    \a before. Returns an iterator pointing at the inserted item.
*/

/*! \fn QVector::iterator QVector::insert(iterator before, int count, const T &value)

    Inserts \a count copies of \a value in front of the item pointed to
    by the iterator \a before. Returns an iterator pointing at the
    first of the inserted items.
*/

/*! \fn void QVector::replace(int i, const T &value)

    Replaces the item at index position \a i with \a value.

    \a i must be a valid index position in the vector (i.e., 0 <= \a
    i < size()).

    \sa operator[](), remove()
*/

/*! \fn void QVector::remove(int i)

    \overload

    Removes the element at index position \a i.

    \sa insert(), replace(), fill()
*/

/*! \fn void QVector::remove(int i, int count)

    \overload

    Removes \a count elements from the middle of the vector, starting at
    index position \a i.

    \sa insert(), replace(), fill()
*/

/*! \fn QVector &QVector::fill(const T &value, int size = -1)

    Assigns \a value to all items in the vector. If \a size is
    different from -1 (the default), the vector is resized to size \a
    size beforehand.

    Example:
    \code
        QVector<QString> vector(3);
        vector.fill("Yes");
        // vector: ["Yes", "Yes", "Yes"]

        vector.fill("oh", 5);
        // vector: ["oh", "oh", "oh", "oh", "oh"]
    \endcode

    \sa resize()
*/

/*! \fn int QVector::indexOf(const T &value, int from = 0) const

    Returns the index position of the first occurrence of \a value in
    the vector, searching forward from index position \a from.
    Returns -1 if no item matched.

    Example:
    \code
        QVector<QString> vector;
        vector << "A" << "B" << "C" << "B" << "A";
        vector.indexOf("B");            // returns 1
        vector.indexOf("B", 1);         // returns 1
        vector.indexOf("B", 2);         // returns 3
        vector.indexOf("X");            // returns -1
    \endcode

    This function requires the value type to have an implementation of
    \c operator==().

    \sa lastIndexOf(), contains()
*/

/*! \fn int QVector::lastIndexOf(const T &value, int from = -1) const

    Returns the index position of the last occurrence of the value \a
    value in the vector, searching backward from index position \a
    from. If \a from is -1 (the default), the search starts at the
    last item. Returns -1 if no item matched.

    Example:
    \code
        QList<QString> vector;
        vector << "A" << "B" << "C" << "B" << "A";
        vector.lastIndexOf("B");        // returns 3
        vector.lastIndexOf("B", 3);     // returns 3
        vector.lastIndexOf("B", 2);     // returns 1
        vector.lastIndexOf("X");        // returns -1
    \endcode

    This function requires the value type to have an implementation of
    \c operator==().

    \sa indexOf()
*/

/*! \fn bool QVector::contains(const T &value) const

    Returns true if the vector contains an occurrence of \a value;
    otherwise returns false.

    This function requires the value type to have an implementation of
    \c operator==().

    \sa indexOf(), count()
*/

/*! \fn int QVector::count(const T &value) const

    Returns the number of occurrences of \a value in the vector.

    This function requires the value type to have an implementation of
    \c operator==().

    \sa contains(), indexOf()
*/

/*! \fn int QVector::count() const

    \overload

    Same as size().
*/

/*! \fn QVector::iterator QVector::begin()

    Returns an \l{STL-style iterator} pointing to the first item in
    the vector.

    \sa constBegin(), end()
*/

/*! \fn QVector::const_iterator QVector::begin() const

    \overload
*/

/*! \fn QVector::const_iterator QVector::constBegin() const

    Returns a const \l{STL-style iterator} pointing to the first item
    in the vector.

    \sa begin(), constEnd()
*/

/*! \fn QVector::iterator QVector::end()

    Returns an \l{STL-style iterator} pointing to the imaginary item
    after the last item in the vector.

    \sa begin(), constEnd()
*/

/*! \fn QVector::const_iterator QVector::end() const

    \overload
*/

/*! \fn QVector::const_iterator QVector::constEnd() const

    Returns a const \l{STL-style iterator} pointing to the imaginary
    item after the last item in the vector.

    \sa constBegin(), end()
*/

/*! \fn QVector::iterator QVector::erase(iterator pos)

    Removes the item pointed to by the iterator \a pos from the
    vector, and returns an iterator to the next item in the vector
    (which may be end()).

    \sa insert(), remove()
*/

/*! \fn QVector::iterator QVector::erase(iterator begin, iterator end)

    \overload

    Removes all the items from \a begin up to (but not including) \a
    end. Returns an iterator to the same item that \a end referred to
    before the call.
*/

/*! \fn T& QVector::first()

    Returns a reference to the first item in the vector. This
    function assumes that the vector isn't empty.

    \sa last(), isEmpty()
*/

/*! \fn const T& QVector::first() const

    \overload
*/

/*! \fn T& QVector::last()

    Returns a reference to the last item in the vector. This function
    assumes that the vector isn't empty.

    \sa first(), isEmpty()
*/

/*! \fn const T& QVector::last() const

    \overload
*/

/*! \fn T QVector::value(int i) const

    Returns the value at index position \a i in the vector.

    If the index \a i is out of bounds, the function returns
    a \l{default-constructed value}. If you are certain that
    \a i is within bounds, you can use at() instead, which is slightly
    faster.

    \sa at(), operator[]()
*/

/*! \fn T QVector::value(int i, const T &defaultValue) const

    \overload

    If the index \a i is out of bounds, the function returns
    \a defaultValue.
*/

/*! \fn void QVector::push_back(const T &value)

    This function is provided for STL compatibility. It is equivalent
    to append(\a value).
*/

/*! \fn void QVector::push_front(const T &value)

    This function is provided for STL compatibility. It is equivalent
    to prepend(\a value).
*/

/*! \fn void QVector::pop_front()

    This function is provided for STL compatibility. It is equivalent
    to erase(begin()).
*/

/*! \fn void QVector::pop_back()

    This function is provided for STL compatibility. It is equivalent
    to erase(end() - 1).
*/

/*! \fn T& QVector::front()

    This function is provided for STL compatibility. It is equivalent
    to first().
*/

/*! \fn QVector::const_reference QVector::front() const

    \overload
*/

/*! \fn QVector::reference QVector::back()

    This function is provided for STL compatibility. It is equivalent
    to last().
*/

/*! \fn QVector::const_reference QVector::back() const

    \overload
*/

/*! \fn bool QVector::empty() const

    This function is provided for STL compatibility. It is equivalent
    to isEmpty().
*/

/*! \fn QVector &QVector::operator+=(const QVector &other)

    Appends the items of the \a other vector to this vector and
    returns a reference to this vector.

    \sa operator+(), append()
*/

/*! \fn void QVector::operator+=(const T &value)

    \overload

    Appends \a value to the vector.

    \sa append(), operator<<()
*/

/*! \fn QVector QVector::operator+(const QVector &other) const

    Returns a vector that contains all the items in this vector
    followed by all the items in the \a other vector.

    \sa operator+=()
*/

/*! \fn QVector &QVector::operator<<(const T &value)

    Appends \a value to the vector and returns a reference to this
    vector.

    \sa append(), operator+=()
*/

/*! \fn QVector &QVector::operator<<(const QVector &other)

    Appends \a other to the vector and returns a reference to the
    vector.
*/

/*! \typedef QVector::iterator

    The QVector::iterator typedef provides an STL-style non-const
    iterator for QVector and QStack.

    QVector provides both \l{STL-style iterators} and \l{Java-style
    iterators}. The STL-style non-const iterator is simply a typedef
    for "T *" (pointer to T).

    \sa QVector::begin(), QVector::end(), QVector::const_iterator, QMutableVectorIterator
*/

/*! \typedef QVector::const_iterator

    The QVector::const_iterator typedef provides an STL-style const
    iterator for QVector and QStack.

    QVector provides both \l{STL-style iterators} and \l{Java-style
    iterators}. The STL-style const iterator is simply a typedef for
    "const T *" (pointer to const T).

    \sa QVector::constBegin(), QVector::constEnd(), QVector::iterator, QVectorIterator
*/

/*! \typedef QVector::Iterator

    Qt-style synonym for QVector::iterator.
*/

/*! \typedef QVector::ConstIterator

    Qt-style synonym for QVector::const_iterator.
*/

/*! \typedef QVector::const_pointer

    \internal
*/

/*! \typedef QVector::const_reference

    \internal
*/

/*! \typedef QVector::difference_type

    \internal
*/

/*! \typedef QVector::pointer

    \internal
*/

/*! \typedef QVector::reference

    \internal
*/

/*! \typedef QVector::size_type

    \internal
*/

/*! \typedef QVector::value_type

    \internal
*/

/*! \fn QList<T> QVector<T>::toList() const

    Returns a QList object with the data contained in this QVector.

    Example:

    \code
        QVector<double> vect;
        vect << "red" << "green" << "blue" << "black";

        QList<double> list = vect.toList();
        // list: ["red", "green", "blue", "black"]
    \endcode

    \sa fromList(), QList::fromVector()
*/

/*! \fn QVector<T> QVector<T>::fromList(const QList<T> &list)

    Returns a QVector object with the data contained in \a list.

    Example:

    \code
        QStringList list;
        list << "Sven" << "Kim" << "Ola";

        QVector<QString> vect = QVector<QString>::fromList(list);
        // vect: ["Sven", "Kim", "Ola"]
    \endcode

    \sa toList(), QList::toVector()
*/

/*! \fn QVector<T> QVector<T>::fromStdVector(const std::vector<T> &vector)

    Returns a QVector object with the data contained in \a vector. The
    order of the elements in the QVector is the same as in \a vector.

    Example:

    \code
        std::vector<double> stdvector;
        vector.push_back(1.2);
        vector.push_back(0.5);
        vector.push_back(3.14);

        QVector<double> vector = QVector<double>::fromStdVector(stdvector);
    \endcode

    \sa toStdVector(), QList::fromStdList()
*/

/*! \fn std::vector<T> QVector<T>::toStdVector() const

    Returns a std::vector object with the data contained in this QVector.
    Example:

    \code
        QVector<double> vector;
        vector << 1.2 << 0.5 << 3.14;

        std::vector<double> stdvector = vector.toStdVector();
    \endcode

    \sa fromStdVector(), QList::toStdList()
*/

/*! \fn QDataStream &operator<<(QDataStream &out, const QVector<T> &vector)
    \relates QVector

    Writes the vector \a vector to stream \a out.

    This function requires the value type to implement \c operator<<().

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

/*! \fn QDataStream &operator>>(QDataStream &in, QVector<T> &vector)
    \relates QVector

    Reads a vector from stream \a in into \a vector.

    This function requires the value type to implement \c operator>>().

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
