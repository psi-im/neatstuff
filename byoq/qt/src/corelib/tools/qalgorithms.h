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

#ifndef QALGORITHMS_H
#define QALGORITHMS_H

#include <QtCore/qglobal.h>

QT_MODULE(Core)

/*
    Warning: The contents of QAlgorithmsPrivate is not a part of the public Qt API
    and may be changed from version to version or even be completely removed.
*/
namespace QAlgorithmsPrivate {

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan);
template <typename BiIterator, typename T>
inline void qSortHelper(BiIterator begin, BiIterator end, const T &dummy);

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan);
template <typename BiIterator, typename T>
inline void qStableSortHelper(BiIterator, BiIterator, const T &);

}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator qCopy(InputIterator begin, InputIterator end, OutputIterator dest)
{
    while (begin != end)
        *dest++ = *begin++;
    return dest;
}

template <typename BiIterator1, typename BiIterator2>
inline BiIterator2 qCopyBackward(BiIterator1 begin, BiIterator1 end, BiIterator2 dest)
{
    while (begin != end)
        *--dest = *--end;
    return dest;
}

template <typename InputIterator1, typename InputIterator2>
inline bool qEqual(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
{
    for (; first1 != last1; ++first1, ++first2)
        if (!(*first1 == *first2))
            return false;
    return true;
}

template <typename ForwardIterator, typename T>
inline void qFill(ForwardIterator first, ForwardIterator last, const T &val)
{
    for (; first != last; ++first)
        *first = val;
}

template <typename InputIterator, typename T>
inline InputIterator qFind(InputIterator first, InputIterator last, const T &val)
{
    while (first != last && !(*first == val))
        ++first;
    return first;
}

template <typename InputIterator, typename T, typename Size>
inline void qCount(InputIterator first, InputIterator last, const T &value, Size &n)
{
    for (; first != last; ++first)
        if (*first == value)
            ++n;
}

template <typename T>
inline void qSwap(T &value1, T &value2)
{
    if (!QTypeInfo<T>::isComplex || QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic) {
        T t = value1;
        value1 = value2;
        value2 = t;
    } else {
        const void * const t = reinterpret_cast<const void * const &>(value1);
        const_cast<const void *&>(reinterpret_cast<const void * const &>(value1)) =
            reinterpret_cast<const void * const &>(value2);
        const_cast<const void *&>(reinterpret_cast<const void * const &>(value2)) = t;
    }
}

template <typename T>
class qLess
{
public:
    inline bool operator()(const T &t1, const T &t2) const
    {
        return (t1 < t2);
    }
};

template <typename T>
class qGreater
{
public:
    inline bool operator()(const T &t1, const T &t2) const
    {
        return (t2 < t1);
    }
};

template <typename BiIterator>
inline void qSort(BiIterator start, BiIterator end)
{
    if (start != end)
        QAlgorithmsPrivate::qSortHelper(start, end, *start);
}

template <typename BiIterator, typename LessThan>
inline void qSort(BiIterator start, BiIterator end, LessThan lessThan)
{
    if (start != end)
        QAlgorithmsPrivate::qSortHelper(start, end, *start, lessThan);
}

template<typename Container>
inline void qSort(Container &c)
{
#ifdef Q_CC_BOR
    // Work around Borland 5.5 optimizer bug
    c.detach();
#endif
    if (!c.empty())
        QAlgorithmsPrivate::qSortHelper(c.begin(), c.end(), *c.begin());
}

template <typename BiIterator>
inline void qStableSort(BiIterator start, BiIterator end)
{
    if (start != end)
        QAlgorithmsPrivate::qStableSortHelper(start, end, *start);
}

template <typename BiIterator, typename LessThan>
inline void qStableSort(BiIterator start, BiIterator end, LessThan lessThan)
{
    if (start != end)
        QAlgorithmsPrivate::qStableSortHelper(start, end, *start, lessThan);
}

template<typename Container>
inline void qStableSort(Container &c)
{
#ifdef Q_CC_BOR
    // Work around Borland 5.5 optimizer bug
    c.detach();
#endif
    if (!c.empty())
        QAlgorithmsPrivate::qStableSortHelper(c.begin(), c.end(), *c.begin());
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qLowerBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    RandomAccessIterator middle;
    int n = end - begin;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = begin + half;
        if (*middle < value) {
            begin = middle + 1;
            n -= half + 1;
        } else {
            n = half;
        }
    }
    return begin;
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qUpperBound(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    RandomAccessIterator middle;
    int n = end - begin;
    int half;

    while (n > 0) {
        half = n >> 1;
        middle = begin + half;
        if (value < *middle) {
            n = half;
        } else {
            begin = middle + 1;
            n -= half + 1;
        }
    }
    return begin;
}

template <typename RandomAccessIterator, typename T>
Q_OUTOFLINE_TEMPLATE RandomAccessIterator qBinaryFind(RandomAccessIterator begin, RandomAccessIterator end, const T &value)
{
    int l = 0;
    int r = end - begin - 1;
    if (r < 0)
        return end;
    int i = (l + r + 1) / 2;

    while (r != l) {
        if (value < begin[i])
            r = i - 1;
        else
            l = i;
        i = (l + r + 1) / 2;
    }
    if (begin[i] < value || value < begin[i])
        return end;
    else
        return begin + i;
}

template <typename ForwardIterator>
Q_OUTOFLINE_TEMPLATE void qDeleteAll(ForwardIterator begin, ForwardIterator end)
{
    while (begin != end) {
        delete *begin;
        ++begin;
    }
}

template <typename Container>
inline void qDeleteAll(const Container &c)
{
    qDeleteAll(c.begin(), c.end());
}

/*
    Warning: The contents of QAlgorithmsPrivate is not a part of the public Qt API
    and may be changed from version to version or even be completely removed.
*/
namespace QAlgorithmsPrivate {

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan)
{
top:
    int span = end - start;
    if (span < 2)
        return;

    --end;
    BiIterator low = start, high = end - 1;
    BiIterator pivot = start + span / 2;

    if (lessThan(*end, *start))
        qSwap(*end, *start);
    if (span == 2)
        return;

    if (lessThan(*pivot, *start))
        qSwap(*pivot, *start);
    if (lessThan(*end, *pivot))
        qSwap(*end, *pivot);
    if (span == 3)
        return;

    qSwap(*pivot, *end);

    while (low < high) {
        while (low < high && lessThan(*low, *end))
            ++low;

        while (high > low && lessThan(*end, *high))
            --high;

        if (low < high) {
            qSwap(*low, *high);
            ++low;
            --high;
        } else {
            break;
        }
    }

    if (lessThan(*low, *end))
        ++low;

    qSwap(*end, *low);
    qSortHelper(start, low, t, lessThan);

    start = low + 1;
    ++end;
    goto top;
}

template <typename BiIterator, typename T>
inline void qSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    qSortHelper(begin, end, dummy, qLess<T>());
}

template <typename BiIterator, typename T, typename LessThan>
Q_OUTOFLINE_TEMPLATE void qStableSortHelper(BiIterator start, BiIterator end, const T &t, LessThan lessThan)
{
    const int span = end - start;
    if (span < 2)
       return;
        
    // Split in half and sort halves.
    BiIterator middle = start + span / 2;
    qStableSortHelper(start, middle, t, lessThan);
    qStableSortHelper(middle, end, t, lessThan);
    
    // Merge
    BiIterator lo = start;
    BiIterator hi = middle;
    
    while (lo != middle && hi != end) {
        if (!lessThan(*hi, *lo)) {
            ++lo; // OK, *lo is in its correct position
        } else {
            // Move *hi to lo's position, shift values
            // between lo and hi - 1 one place up.
            T value = *hi;
            for (BiIterator i = hi; i != lo; --i) {
                *i = *(i-1); 
            }
            *lo = value;
            ++hi;
            ++lo;
            ++middle;
        } 
    }
}

template <typename BiIterator, typename T>
inline void qStableSortHelper(BiIterator begin, BiIterator end, const T &dummy)
{
    qStableSortHelper(begin, end, dummy, qLess<T>());
}

} //namespace QAlgorithmsPrivate

#endif // QALGORITHMS_H
