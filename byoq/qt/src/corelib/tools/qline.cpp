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

#include "qline.h"
#include "qdebug.h"
#include "qdatastream.h"
#include <private/qnumeric_p.h>

#include <math.h>

/*!
    \class QLine

    \brief The QLine class provides a two-dimensional vector that
    uses integer point accuracy for coordinates.

    A QLine describes a finite length line (or line segment) on a
    two-dimensional surface.  The start and end points of the line are
    specified using integer point coordinates.

    Use isNull() to determine whether the QLine represents a valid line
    or a null line.

    \table
    \row
        \o \inlineimage qline-point.png
        \o \inlineimage qline-coordinates.png
    \endtable

    The positions of the line's end points can be found with the p1(),
    x1(), y1(), p2(), x2(), and y2() functions. The horizontal and vertical
    components of the line are returned by the dx() and dy() functions.

    The line can be translated along the length of another line with the
    translate() function.

    \sa QPoint QSize QRect QLineF
*/

/*!
    \fn QLine::QLine()

    Constructs a null line.
*/

/*!
    \fn QLine::QLine(const QPoint &pt1, const QPoint &pt2)

    Constructs a line object that represents the line between \a pt1 and
    \a pt2.
*/

/*!
    \fn QLine::QLine(int x1, int y1, int x2, int y2)

    Constructs a line object that represents the line between (\a x1, \a y1) and
    (\a x2, \a y2).
*/

/*!
    \fn bool QLine::isNull() const

    Returns true if the line is not set up with valid start and end point;
    otherwise returns false.
*/

/*!
    \fn QPoint QLine::p1() const

    Returns the line's start point.

    \sa x1() y1() p2()
*/

/*!
    \fn QPoint QLine::p2() const

    Returns the line's end point.

    \sa x2() y2() p1()
*/

/*!
    \fn int QLine::x1() const

    Returns the x-coordinate of the line's start point.

    \sa y1() x2() p1()
*/

/*!
    \fn int QLine::y1() const

    Returns the y-coordinate of the line's start point.

    \sa x1() y2() p1()
*/

/*!
    \fn int QLine::x2() const

    Returns the x-coordinate of the line's end point.

    \sa y2() x1() p2()
*/

/*!
    \fn int QLine::y2() const

    Returns the y-coordinate of the line's end point.

    \sa x2() y1() p2()
*/

/*!
    \fn int QLine::dx() const

    Returns the horizontal component of the line's vector.

    \sa dy()
*/

/*!
    \fn int QLine::dy() const

    Returns the vertical component of the line's vector.

    \sa dx()
*/

/*!
    \fn bool QLine::operator!=(const QLine &other) const

    Returns true if \a other is not the same as this line.

    A line is different from another line if any of their points are
    different or their order is different.
*/

/*!
    \fn bool QLine::operator==(const QLine &other) const

    Returns true if \a other is the same line as this line.

    A line is identical to another line if the points are the same and their
    order is the same.
*/

/*!
    \fn void QLine::translate(const QPoint &point)

    Translates this line with the \a point given.
*/

/*!
    \fn void QLine::translate(int dx, int dy)

    \overload

    Translates this line the distance \a dx and \a dy.
*/

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QLine &p)
{
    d << "QLine(" << p.p1() << "," << p.p2() << ")";
    return d;
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \relates QLine

    Writes the \a line to the \a stream and returns a reference to
    the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &stream, const QLine &line)
{
    stream << line.p1() << line.p2();
    return stream;
}

/*!
    \relates QLine

    Reads a QLine from the \a stream into the \a line and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &stream, QLine &line)
{
    QPoint p1, p2;
    stream >> p1;
    stream >> p2;
    line = QLine(p1, p2);

    return stream;
}

#endif // QT_NO_DATASTREAM


#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

/*!
    \class QLineF

    \brief The QLineF class provides a two-dimensional vector that
    uses floating point accuracy for coordinates.

    A QLineF describes a finite length line (or line segment) on a
    two-dimensional surface. QLineF provides a variant of the QLine
    class that defines the start and end points of the line using
    floating point coordinates.

    Use isNull() to determine whether the QLineF represents a valid line
    or a null line.

    \table
    \row
        \o \inlineimage qline-point.png
        \o \inlineimage qline-coordinates.png
    \endtable

    The positions of the line's end points can be found with the p1(), x1(),
    y1(), p2(), x2(), and y2() functions. The horizontal and vertical
    components of the line are returned by the dx() and dy() functions.

    Convenience functions are provided for finding the lines's length(),
    the unitVector() along the line, whether two lines intersect(), and
    the angle() between two lines. The line's length can be changed using
    setLength().

    The line can be translated along the length of another line with the
    moveBy() function, and can be traversed using a parameter with the
    pointAt() function.

    \sa QPointF QSizeF QRectF QLine
*/

/*!
    \enum QLineF::IntersectType

    \value NoIntersection Indicates that the lines do not intersect;
    i.e. they are parallel.

    \value UnboundedIntersection The two lines intersect,
    but not within the range defined by their lengths. This will be
    the case if the lines are not parallel.

    \img qlinef-unbounded.png

    \value BoundedIntersection The two lines intersect with each other
    within the start and end points of each line.

    \img qlinef-bounded.png

*/

/*!
    \fn QLineF::QLineF()

    Constructs a null line.
*/

/*!
    \fn QLineF::QLineF(const QPointF &pt1, const QPointF &pt2)

    Constructs a line object that represents the line between \a pt1 and
    \a pt2.
*/

/*!
    \fn QLineF::QLineF(qreal x1, qreal y1, qreal x2, qreal y2)

    Constructs a line object that represents the line between (\a x1, \a y1) and
    (\a x2, \a y2).
*/

/*!
    \fn QLineF::QLineF(const QLine &line)

    Construct a QLineF from a integer-based QLine \a line.
*/

/*!
    Returns true if the line is not set up with valid start and end point;
    otherwise returns false.
*/

bool QLineF::isNull() const
{
    return (qFuzzyCompare(pt1.x(), pt2.x()) && qFuzzyCompare(pt1.y(), pt2.y())) ? true : false;
}


/*!
    \fn QPointF QLineF::p1() const

    Returns the line's start point.

    \sa x1() y1() p2()
*/

/*!
    \fn QPointF QLineF::p2() const

    Returns the line's end point.

    \sa x2() y2() p1()
*/

/*!
    \fn QLine QLineF::toLine() const

    Returns a QLine. The returned QLine's starting and end points are rounded to
    the nearest integer.

*/
/*!
    \fn qreal QLineF::x1() const

    Returns the x-coordinate of the line's start point.

    \sa y1() p1() x2()
*/

/*!
    \fn qreal QLineF::y1() const

    Returns the y-coordinate of the line's start point.

    \sa x1() p1() x2()
*/

/*!
    \fn qreal QLineF::x2() const

    Returns the x-coordinate of the line's end point.

    \sa y2() p2() x1()
*/

/*!
    \fn qreal QLineF::y2() const

    Returns the y-coordinate of the line's end point.

    \sa x2() p2() y1()
*/

/*!
    \fn qreal QLineF::dx() const

    Returns the horizontal component of the line's vector.

    \sa dy() pointAt()
*/

/*!
    \fn qreal QLineF::dy() const

    Returns the vertical component of the line's vector.

    \sa dx() pointAt()
*/

/*!
    \fn QLineF::setLength(qreal length)

    Sets the \a length of the line.

    \sa length()
*/

/*!
    \fn QLineF QLineF::normalVector() const

    Returns a line that is perpendicular to this line with the same starting
    point and length.

    \sa unitVector()

    \img qlinef-normalvector.png
*/

/*!
    \fn bool QLineF::operator!=(const QLineF &other) const

    Returns true if \a other is not the same as this line.

    A line is different from another line if any of their points are
    different or their order is different.
*/

/*!
    \fn bool QLineF::operator==(const QLineF &other) const

    Returns true if \a other is the same line as this line.

    A line is identical if the two points are the same and their
    order is the same.
*/

/*!
  \fn qreal QLineF::pointAt(qreal t) const

  Returns the point at the parameterized position \a t, where
  the start and end point are defined to be at positions t=0 and t=1.

  \sa dx() dy()
*/

/*!
    Returns the length of the line.

    \sa setLength()
*/
qreal QLineF::length() const
{
    qreal x = pt2.x() - pt1.x();
    qreal y = pt2.y() - pt1.y();
    return sqrt(x*x + y*y);
}


/*!
    Returns a normalized version of this line, starting at the same
    point as this line. A normalized line is a line of unit length
    (length() is equal to 1.0).

    \sa normalVector()
*/
QLineF QLineF::unitVector() const
{
    qreal x = pt2.x() - pt1.x();
    qreal y = pt2.y() - pt1.y();

    qreal len = sqrt(x*x + y*y);
    QLineF f(p1(), QPointF(pt1.x() + x/len, pt1.y() + y/len));

#ifndef QT_NO_DEBUG
    if (qAbs(f.length() - 1) >= 0.001)
        qWarning("QLine::unitVector(), new line does not have length of 1");
#endif

    return f;
}

#define SAME_SIGNS(a, b) ((a) * (b) >= 0)

// Line intersection algorithm, copied from Graphics Gems II
static bool qt_linef_intersect(qreal x1, qreal y1, qreal x2, qreal y2,
                               qreal x3, qreal y3, qreal x4, qreal y4)
{
    qreal a1, a2, b1, b2, c1, c2; /* Coefficients of line eqns. */
    qreal r1, r2, r3, r4;         /* 'Sign' values */

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;

    r3 = a1 * x3 + b1 * y3 + c1;
    r4 = a1 * x4 + b1 * y4 + c1;

    if ( r3 != 0 && r4 != 0 && SAME_SIGNS( r3, r4 ))
        return false;

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;

    r1 = a2 * x1 + b2 * y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;

    if ( r1 != 0 && r2 != 0 && SAME_SIGNS( r1, r2 ))
        return false;

    return true;
}

/*!
    \fn QLineF::IntersectType QLineF::intersect(const QLineF &other, QPointF *intersectionPoint) const

    Returns a value indicating whether or not this line intersects the
    \a other line. By passing a valid pointer as \a intersectionPoint, it
    is possible to get the actual intersection point. The intersection
    point is undefined if the lines are parallel.
*/

QLineF::IntersectType QLineF::intersect(const QLineF &l, QPointF *intersectionPoint) const
{
    if (isNull() || l.isNull()
        || !qIsFinite(pt1.x()) || !qIsFinite(pt1.y()) || !qIsFinite(pt2.x()) || !qIsFinite(pt2.y())
        || !qIsFinite(l.pt1.x()) || !qIsFinite(l.pt1.y()) || !qIsFinite(l.pt2.x()) || !qIsFinite(l.pt2.y()))
        return NoIntersection;

    QPointF isect;
    IntersectType type = qt_linef_intersect(pt1.x(), pt1.y(), pt2.x(), pt2.y(),
                                            l.x1(), l.y1(), l.x2(), l.y2())
                         ? BoundedIntersection : UnboundedIntersection;

    // For special case where one of the lines are vertical
    if (dx() == 0 && l.dx() == 0) {
        type = NoIntersection;
    } else if (dx() == 0) {
        qreal la = l.dy() / l.dx();
        isect = QPointF(pt1.x(), la * pt1.x() + l.y1() - la*l.x1());
    } else if (l.dx() == 0) {
        qreal ta = dy() / dx();
        isect = QPointF(l.x1(), ta * l.x1() + y1() - ta*x1());
    } else {
        qreal ta = dy()/dx();
        qreal la = l.dy()/l.dx();
        if (ta == la) // no intersection
            return NoIntersection;

        qreal x = ( - l.y1() + la * l.x1() + pt1.y() - ta * pt1.x() ) / (la - ta);
        isect = QPointF(x, ta*(x - pt1.x()) + pt1.y());
    }
    if (intersectionPoint)
        *intersectionPoint = isect;
    return type;
}

/*!
    \fn void QLineF::translate(const QPointF &point)

    Translates this line with the \a point given.
*/

/*!
    \fn void QLineF::translate(qreal dx, qreal dy)

    \overload

    Translates this line the distance \a dx and \a dy.
*/

/*!
  \fn qreal QLineF::angle(const QLineF &line) const

  Returns the smallest angle between the given \a line and this line, not
  taking into account whether the lines intersect or not. The angle is
  specified in degrees.

  \sa intersect()
*/
qreal QLineF::angle(const QLineF &l) const
{
    if (isNull() || l.isNull())
        return 0;
    qreal cos_line = (dx()*l.dx() + dy()*l.dy()) / (length()*l.length());
    qreal rad = 0;
    // only accept cos_line in the range [-1,1], if it is outside, use 0 (we return 0 rather than PI for those cases)
    if (cos_line >= -1.0 && cos_line <= 1.0) rad = acos( cos_line );
    return rad * 360 / M_2PI;
}


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QLineF &p)
{
    d << "QLineF(" << p.p1() << "," << p.p2() << ")";
    return d;
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \relates QLineF

    Writes the \a line to the \a stream and returns a reference to
    the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &stream, const QLineF &line)
{
    stream << line.p1() << line.p2();
    return stream;
}

/*!
    \relates QLineF

    Reads a QLineF from the \a stream into the \a line and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &stream, QLineF &line)
{
    QPointF start, end;
    stream >> start;
    stream >> end;
    line = QLineF(start, end);

    return stream;
}

#endif // QT_NO_DATASTREAM
