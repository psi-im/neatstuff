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

#ifndef QDATETIME_H
#define QDATETIME_H

#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>

QT_MODULE(Core)

class Q_CORE_EXPORT QDate
{
public:
    QDate() { jd = 0; }
    QDate(int y, int m, int d);

    bool isNull() const { return jd == 0; }
    bool isValid() const;

    int year() const;
    int month() const;
    int day() const;
    int dayOfWeek() const;
    int dayOfYear() const;
    int daysInMonth() const;
    int daysInYear() const;
    int weekNumber(int *yearNum = 0) const;

#ifndef QT_NO_TEXTDATE
#ifdef QT3_SUPPORT
    static QT3_SUPPORT QString monthName(int month) { return shortMonthName(month); }
    static QT3_SUPPORT QString dayName(int weekday) { return shortDayName(weekday); }
#endif
    static QString shortMonthName(int month);
    static QString shortDayName(int weekday);
    static QString longMonthName(int month);
    static QString longDayName(int weekday);
#endif // QT_NO_TEXTDATE
#ifndef QT_NO_DATESTRING
    QString toString(Qt::DateFormat f = Qt::TextDate) const;
    QString toString(const QString &format) const;
#endif
    bool setYMD(int y, int m, int d);

    QDate addDays(int days) const;
    QDate addMonths(int months) const;
    QDate addYears(int years) const;
    int daysTo(const QDate &) const;

    bool operator==(const QDate &other) const { return jd == other.jd; }
    bool operator!=(const QDate &other) const { return jd != other.jd; }
    bool operator<(const QDate &other) const { return jd < other.jd; }
    bool operator<=(const QDate &other) const { return jd <= other.jd; }
    bool operator>(const QDate &other) const { return jd > other.jd; }
    bool operator>=(const QDate &other) const { return jd >= other.jd; }

    static QDate currentDate();
#ifndef QT_NO_DATESTRING
    static QDate fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);
    static QDate fromString(const QString &s, const QString &format);
#endif
    static bool isValid(int y, int m, int d);
    static bool isLeapYear(int year);
#ifdef QT3_SUPPORT
    inline static QT3_SUPPORT bool leapYear(int year) { return isLeapYear(year); }
#endif

    static uint gregorianToJulian(int y, int m, int d);
    static void julianToGregorian(uint jd, int &y, int &m, int &d);

#ifdef QT3_SUPPORT
    static QT3_SUPPORT QDate currentDate(Qt::TimeSpec spec);
#endif

    static inline QDate fromJulianDay(int jd) { QDate d; d.jd = jd; return d; }
    inline int toJulianDay() const { return jd; }

private:
    uint jd;

    friend class QDateTime;
    friend class QDateTimePrivate;
#ifndef QT_NO_DATASTREAM
    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDate &);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDate &);
#endif
};
Q_DECLARE_TYPEINFO(QDate, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QTime
{
public:
    QTime(): mds(NullTime) {}
    QTime(int h, int m, int s = 0, int ms = 0);

    bool isNull() const { return mds == NullTime; }
    bool isValid() const;

    int hour() const;
    int minute() const;
    int second() const;
    int msec() const;
#ifndef QT_NO_DATESTRING
    QString toString(Qt::DateFormat f = Qt::TextDate) const;
    QString toString(const QString &format) const;
#endif
    bool setHMS(int h, int m, int s, int ms = 0);

    QTime addSecs(int secs) const;
    int secsTo(const QTime &) const;
    QTime addMSecs(int ms) const;
    int msecsTo(const QTime &) const;

    bool operator==(const QTime &other) const { return mds == other.mds; }
    bool operator!=(const QTime &other) const { return mds != other.mds; }
    bool operator<(const QTime &other) const { return mds < other.mds; }
    bool operator<=(const QTime &other) const { return mds <= other.mds; }
    bool operator>(const QTime &other) const { return mds > other.mds; }
    bool operator>=(const QTime &other) const { return mds >= other.mds; }

    static QTime currentTime();
#ifndef QT_NO_DATESTRING
    static QTime fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);
#endif
    static QTime fromString(const QString &s, const QString &format);
    static bool isValid(int h, int m, int s, int ms = 0);

#ifdef QT3_SUPPORT
    static QT3_SUPPORT QTime currentTime(Qt::TimeSpec spec);
    static QT3_SUPPORT QDate currentDate(Qt::TimeSpec spec);
#endif

    void start();
    int restart();
    int elapsed() const;

private:
    enum { NullTime = -1 };
    inline int ds() const { return mds == -1 ? 0 : mds; }
    int mds;

    friend class QDateTime;
    friend class QDateTimePrivate;
#ifndef QT_NO_DATASTREAM
    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QTime &);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QTime &);
#endif
};
Q_DECLARE_TYPEINFO(QTime, Q_MOVABLE_TYPE);

class QDateTimePrivate;

class Q_CORE_EXPORT QDateTime
{
public:
    QDateTime();
    explicit QDateTime(const QDate &);
    QDateTime(const QDate &, const QTime &, Qt::TimeSpec spec = Qt::LocalTime);
    QDateTime(const QDateTime &other);
    ~QDateTime();

    QDateTime &operator=(const QDateTime &other);

    bool isNull() const;
    bool isValid() const;

    QDate date() const;
    QTime time() const;
    Qt::TimeSpec timeSpec() const;
    uint toTime_t() const;
    void setDate(const QDate &date);
    void setTime(const QTime &time);
    void setTimeSpec(Qt::TimeSpec spec);
    void setTime_t(uint secsSince1Jan1970UTC);
#ifndef QT_NO_DATESTRING
    QString toString(Qt::DateFormat f = Qt::TextDate) const;
    QString toString(const QString &format) const;
#endif
    QDateTime addDays(int days) const;
    QDateTime addMonths(int months) const;
    QDateTime addYears(int years) const;
    QDateTime addSecs(int secs) const;
    QDateTime addMSecs(qint64 msecs) const;
    QDateTime toTimeSpec(Qt::TimeSpec spec) const;
    inline QDateTime toLocalTime() const { return toTimeSpec(Qt::LocalTime); }
    inline QDateTime toUTC() const { return toTimeSpec(Qt::UTC); }
    int daysTo(const QDateTime &) const;
    int secsTo(const QDateTime &) const;

    bool operator==(const QDateTime &other) const;
    inline bool operator!=(const QDateTime &other) const { return !(*this == other); }
    bool operator<(const QDateTime &other) const;
    inline bool operator<=(const QDateTime &other) const { return !(other < *this); }
    inline bool operator>(const QDateTime &other) const { return other < *this; }
    inline bool operator>=(const QDateTime &other) const { return !(*this < other); }

    static QDateTime currentDateTime();
#ifndef QT_NO_DATESTRING
    static QDateTime fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);
#endif
    static QDateTime fromString(const QString &s, const QString &format);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT void setTime_t(uint secsSince1Jan1970UTC, Qt::TimeSpec spec) {
        setTime_t(secsSince1Jan1970UTC);
        if (spec == Qt::UTC)
            *this = toUTC();
    }
    static inline QT3_SUPPORT QDateTime currentDateTime(Qt::TimeSpec spec) {
        if (spec == Qt::LocalTime)
            return currentDateTime();
        else
            return currentDateTime().toUTC();
    }
#endif

private:
    friend class QDateTimePrivate;
    void detach();
    QDateTimePrivate *d;

#ifndef QT_NO_DATASTREAM
    friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDateTime &);
    friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDateTime &);
#endif
};
Q_DECLARE_TYPEINFO(QDateTime, Q_MOVABLE_TYPE);

#ifdef QT3_SUPPORT
inline QDate QDate::currentDate(Qt::TimeSpec spec)
{
    if (spec == Qt::LocalTime)
        return currentDate();
    else
        return QDateTime::currentDateTime().toUTC().date();
}

inline QTime QTime::currentTime(Qt::TimeSpec spec)
{
    if (spec == Qt::LocalTime)
        return currentTime();
    else
        return QDateTime::currentDateTime().toUTC().time();
}
#endif

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDate &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDate &);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QTime &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QTime &);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDateTime &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDateTime &);
#endif // QT_NO_DATASTREAM

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_NO_DATESTRING)
Q_CORE_EXPORT QDebug operator<<(QDebug, const QDate &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QTime &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QDateTime &);
#endif

#endif // QDATETIME_H
