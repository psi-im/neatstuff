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

#include "qplatformdefs.h"

#include "qdatastream.h"
#include "qstring.h"
#include "qlocale.h"
#include "qlocale_p.h"
#include "qnamespace.h"
#include "qdatetime.h"
#include "qstringlist.h"
#if defined(Q_WS_WIN)
#   include "qt_windows.h"
#endif
#if !defined(QWS) && defined(Q_OS_MAC)
#   include "private/qcore_mac_p.h"
#endif
#include "private/qnumeric_p.h"

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#if defined(Q_OS_LINUX) && !defined(__UCLIBC__)
#    include <fenv.h>
#endif

#if !defined(QT_QLOCALE_NEEDS_VOLATILE)
#  if defined(Q_CC_GNU)
#    if  __GNUC__ == 4 && __GNUC_MINOR__ == 0
#      define QT_QLOCALE_NEEDS_VOLATILE
#    elif defined(Q_OS_WIN)
#      define QT_QLOCALE_NEEDS_VOLATILE
#    endif
#  endif
#endif

#if defined(QT_QLOCALE_NEEDS_VOLATILE)
#   define NEEDS_VOLATILE volatile
#else
#   define NEEDS_VOLATILE
#endif

// Sizes as defined by the ISO C99 standard - fallback
#ifndef LLONG_MAX
#   define LLONG_MAX Q_INT64_C(0x7fffffffffffffff)
#endif
#ifndef LLONG_MIN
#   define LLONG_MIN (-LLONG_MAX - Q_INT64_C(1))
#endif
#ifndef ULLONG_MAX
#   define ULLONG_MAX Q_UINT64_C(0xffffffffffffffff)
#endif

#define CONVERSION_BUFF_SIZE 255

#ifndef QT_QLOCALE_USES_FCVT
static char *qdtoa(double d, int mode, int ndigits, int *decpt,
                        int *sign, char **rve, char **digits_str);
static char *_qdtoa( NEEDS_VOLATILE double d, int mode, int ndigits, int *decpt,
                        int *sign, char **rve, char **digits_str);
static double qstrtod(const char *s00, char const **se, bool *ok);
#endif
static qlonglong qstrtoll(const char *nptr, const char **endptr, register int base, bool *ok);
static qulonglong qstrtoull(const char *nptr, const char **endptr, register int base, bool *ok);

#include "qlocale_data_p.h"

static QLocale::Language codeToLanguage(const QString &code)
{
    if (code.length() != 2)
        return QLocale::C;

    ushort uc1 = code.unicode()[0].unicode();
    ushort uc2 = code.unicode()[1].unicode();

    const unsigned char *c = language_code_list;
    for (; *c != 0; c += 2) {
        if (uc1 == c[0] && uc2 == c[1])
            return QLocale::Language((c - language_code_list)/2);
    }

    return QLocale::C;
}

static QLocale::Country codeToCountry(const QString &code)
{
    if (code.length() != 2)
        return QLocale::AnyCountry;

    ushort uc1 = code.unicode()[0].unicode();
    ushort uc2 = code.unicode()[1].unicode();

    const unsigned char *c = country_code_list;
    for (; *c != 0; c += 2) {
        if (uc1 == c[0] && uc2 == c[1])
            return QLocale::Country((c - country_code_list)/2);
    }

    return QLocale::AnyCountry;
}

static QString languageToCode(QLocale::Language language)
{
    if (language == QLocale::C)
        return QLatin1String("C");

    QString code;
    code.resize(2);
    const unsigned char *c = language_code_list + 2*(uint(language));
    code[0] = ushort(c[0]);
    code[1] = ushort(c[1]);
    return code;
}

static QString countryToCode(QLocale::Country country)
{
    if (country == QLocale::AnyCountry)
        return QString();

    QString code;
    code.resize(2);
    const unsigned char *c = country_code_list + 2*(uint(country));
    code[0] = ushort(c[0]);
    code[1] = ushort(c[1]);
    return code;
}

const QLocalePrivate *QLocale::default_d = 0;

QString QLocalePrivate::infinity() const
{
    return QString::fromLatin1("inf");
}

QString QLocalePrivate::nan() const
{
    return QString::fromLatin1("nan");
}

QString QLocalePrivate::month(int index, bool short_format) const
{
    if (index < 0 || index >= 12)
        return QString();

    quint32 idx = short_format ? m_short_month_names_idx : m_long_month_names_idx;
    QStringList month_names = QString::fromUtf8(months_data + idx).split(QLatin1Char(';'));
    return month_names.at(index);
}

QString QLocalePrivate::day(int index, bool short_format) const
{
    if (index < 0 || index >= 7)
        return QString();

    quint32 idx = short_format ? m_short_day_names_idx : m_long_day_names_idx;
    QStringList day_names = QString::fromUtf8(days_data + idx).split(QLatin1Char(';'));
    return day_names.at(index);
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &ds, const QLocale &l)
{
    ds << l.name();
    return ds;
}

QDataStream &operator>>(QDataStream &ds, QLocale &l)
{
    QString s;
    ds >> s;
    l = QLocale(s);
    return ds;
}
#endif

#if defined(Q_OS_WIN)
/* Win95 doesn't have a function to return the ISO lang/country name of the user's locale.
   Instead it can return a "Windows code". This maps windows codes to ISO country names. */

struct WindowsToISOListElt {
    int windows_code;
    char iso_name[6];
};

static const WindowsToISOListElt windows_to_iso_list[] = {
    { 0x0401, "ar_SA" },
    { 0x0402, "bg\0  " },
    { 0x0403, "ca\0  " },
    { 0x0404, "zh_TW" },
    { 0x0405, "cs\0  " },
    { 0x0406, "da\0  " },
    { 0x0407, "de\0  " },
    { 0x0408, "el\0  " },
    { 0x0409, "en_US" },
    { 0x040a, "es\0  " },
    { 0x040b, "fi\0  " },
    { 0x040c, "fr\0  " },
    { 0x040d, "he\0  " },
    { 0x040e, "hu\0  " },
    { 0x040f, "is\0  " },
    { 0x0410, "it\0  " },
    { 0x0411, "ja\0  " },
    { 0x0412, "ko\0  " },
    { 0x0413, "nl\0  " },
    { 0x0414, "no\0  " },
    { 0x0415, "pl\0  " },
    { 0x0416, "pt_BR" },
    { 0x0418, "ro\0  " },
    { 0x0419, "ru\0  " },
    { 0x041a, "hr\0  " },
    { 0x041c, "sq\0  " },
    { 0x041d, "sv\0  " },
    { 0x041e, "th\0  " },
    { 0x041f, "tr\0  " },
    { 0x0420, "ur\0  " },
    { 0x0421, "in\0  " },
    { 0x0422, "uk\0  " },
    { 0x0423, "be\0  " },
    { 0x0425, "et\0  " },
    { 0x0426, "lv\0  " },
    { 0x0427, "lt\0  " },
    { 0x0429, "fa\0  " },
    { 0x042a, "vi\0  " },
    { 0x042d, "eu\0  " },
    { 0x042f, "mk\0  " },
    { 0x0436, "af\0  " },
    { 0x0438, "fo\0  " },
    { 0x0439, "hi\0  " },
    { 0x043e, "ms\0  " },
    { 0x0458, "mt\0  " },
    { 0x0801, "ar_IQ" },
    { 0x0804, "zh_CN" },
    { 0x0807, "de_CH" },
    { 0x0809, "en_GB" },
    { 0x080a, "es_MX" },
    { 0x080c, "fr_BE" },
    { 0x0810, "it_CH" },
    { 0x0812, "ko\0  " },
    { 0x0813, "nl_BE" },
    { 0x0814, "no\0  " },
    { 0x0816, "pt\0  " },
    { 0x081a, "sr\0  " },
    { 0x081d, "sv_FI" },
    { 0x0c01, "ar_EG" },
    { 0x0c04, "zh_HK" },
    { 0x0c07, "de_AT" },
    { 0x0c09, "en_AU" },
    { 0x0c0a, "es\0  " },
    { 0x0c0c, "fr_CA" },
    { 0x0c1a, "sr\0  " },
    { 0x1001, "ar_LY" },
    { 0x1004, "zh_SG" },
    { 0x1007, "de_LU" },
    { 0x1009, "en_CA" },
    { 0x100a, "es_GT" },
    { 0x100c, "fr_CH" },
    { 0x1401, "ar_DZ" },
    { 0x1407, "de_LI" },
    { 0x1409, "en_NZ" },
    { 0x140a, "es_CR" },
    { 0x140c, "fr_LU" },
    { 0x1801, "ar_MA" },
    { 0x1809, "en_IE" },
    { 0x180a, "es_PA" },
    { 0x1c01, "ar_TN" },
    { 0x1c09, "en_ZA" },
    { 0x1c0a, "es_DO" },
    { 0x2001, "ar_OM" },
    { 0x2009, "en_JM" },
    { 0x200a, "es_VE" },
    { 0x2401, "ar_YE" },
    { 0x2409, "en\0  " },
    { 0x240a, "es_CO" },
    { 0x2801, "ar_SY" },
    { 0x2809, "en_BZ" },
    { 0x280a, "es_PE" },
    { 0x2c01, "ar_JO" },
    { 0x2c09, "en_TT" },
    { 0x2c0a, "es_AR" },
    { 0x3001, "ar_LB" },
    { 0x300a, "es_EC" },
    { 0x3401, "ar_KW" },
    { 0x340a, "es_CL" },
    { 0x3801, "ar_AE" },
    { 0x380a, "es_UY" },
    { 0x3c01, "ar_BH" },
    { 0x3c0a, "es_PY" },
    { 0x4001, "ar_QA" },
    { 0x400a, "es_BO" },
    { 0x440a, "es_SV" },
    { 0x480a, "es_HN" },
    { 0x4c0a, "es_NI" },
    { 0x500a, "es_PR" }
};

static const int windows_to_iso_count
    = sizeof(windows_to_iso_list)/sizeof(WindowsToISOListElt);

static const char *winLangCodeToIsoName(int code)
{
    int cmp = code - windows_to_iso_list[0].windows_code;
    if (cmp < 0)
        return 0;

    if (cmp == 0)
        return windows_to_iso_list[0].iso_name;

    int begin = 0;
    int end = windows_to_iso_count;

    while (end - begin > 1) {
        uint mid = (begin + end)/2;

        const WindowsToISOListElt *elt = windows_to_iso_list + mid;
        int cmp = code - elt->windows_code;
        if (cmp < 0)
            end = mid;
        else if (cmp > 0)
            begin = mid;
        else
            return elt->iso_name;
    }

    return 0;

}

static QString winIso639LangName()
{
    QString result;

    // Windows returns the wrong ISO639 for some languages, we need to detect them here using
    // the language code
    QString lang_code;
    QT_WA({
        TCHAR out[256];
        if (GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, out, 255))
            lang_code = QString::fromUtf16((ushort*)out);
    } , {
        char out[256];
        if (GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_ILANGUAGE, out, 255))
            lang_code = QString::fromLocal8Bit(out);
    });

    if (!lang_code.isEmpty()) {
        const char *endptr;
        bool ok;
	QByteArray latin1_lang_code = lang_code.toLatin1();
        int i = qstrtoull(latin1_lang_code, &endptr, 16, &ok);
        if (ok && *endptr == '\0') {
            switch (i) {
                case 0x814:
                    result = QLatin1String("nn"); // Nynorsk
                    break;
                default:
                    break;
            }
        }
    }

    if (!result.isEmpty())
        return result;

    // not one of the problematic languages - do the usual lookup
    QT_WA({
        TCHAR out[256];
        if (GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME , out, 255))
            result = QString::fromUtf16((ushort*)out);
    } , {
        char out[256];
        if (GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, out, 255))
            result = QString::fromLocal8Bit(out);
    });

    return result;
}

static QString winIso3116CtryName()
{
    QString result;

    QT_WA({
        TCHAR out[256];
        if (GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, out, 255))
            result = QString::fromUtf16((ushort*)out);
    } , {
        char out[256];
        if (GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, out, 255))
            result = QString::fromLocal8Bit(out);
    });

    return result;
}

#endif // Q_OS_WIN



QByteArray QLocalePrivate::systemLocaleName()
{
    static QByteArray lang;
    lang = qgetenv("LANG");

#if !defined(QWS) && defined(Q_OS_MAC)
    if (!lang.isEmpty())
        return lang;

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
        QCFType<CFLocaleRef> l = CFLocaleCopyCurrent();
        CFStringRef locale = CFLocaleGetIdentifier(l);
        lang = QCFString::toQString(locale).toLatin1();
    } else
#endif
    {
        char mac_ret[255];
        if(!LocaleRefGetPartString(NULL, kLocaleLanguageMask | kLocaleRegionMask, 255, mac_ret)) {
            lang = mac_ret;
        }
    }
#endif

#if defined(Q_WS_WIN)
    if ( !lang.isEmpty() ) {
        long id = 0;
        bool ok = false;
        id = qstrtoll(lang.data(), 0, 0, &ok);
        if ( !ok || id == 0 || id < INT_MIN || id > INT_MAX )
            return lang;
        else
            return winLangCodeToIsoName( (int)id );
    }

    if (QSysInfo::WindowsVersion == QSysInfo::WV_95) {
        lang = winLangCodeToIsoName(GetUserDefaultLangID());
    } else {
        QString language = winIso639LangName();
        QString country = winIso3116CtryName();
        lang += language.toLatin1();
        if (!country.isEmpty()) {
            lang += '_';
            lang += country.toLatin1();
        }
    }
#endif
    if (lang.isEmpty())
        lang = "C";

    return lang;
}

static const QLocalePrivate *findLocale(QLocale::Language language,
                                            QLocale::Country country)
{
    unsigned language_id = language;
    unsigned country_id = country;

    uint idx = locale_index[language_id];

    const QLocalePrivate *d = locale_data + idx;

    if (idx == 0) // default language has no associated country
        return d;

    if (country == QLocale::AnyCountry)
        return d;

    Q_ASSERT(d->languageId() == language_id);

    while (d->languageId() == language_id
                && d->countryId() != country_id)
        ++d;

    if (d->countryId() == country_id
            && d->languageId() == language_id)
        return d;

    return locale_data + idx;
}

/*!
    \class QLocale
    \brief The QLocale class converts between numbers and their
    string representations in various languages.

    \reentrant
    \ingroup i18n
    \ingroup text
    \mainclass

    QLocale is initialized with a language/country pair in its
    constructor and offers number-to-string and string-to-number
    conversion functions similar to those in QString.

    Example:

    \code
        QLocale egyptian(QLocale::Arabic, QLocale::Egypt);
        QString s1 = egyptian.toString(1.571429E+07, 'e');
        QString s2 = egyptian.toString(10);

        double d = egyptian.toDouble(s1);
        int i = egyptian.toInt(s2);
    \endcode

    QLocale supports the concept of a default locale, which is
    determined from the system's locale settings at application
    startup. The default locale can be changed by calling the
    static member setDefault(). The default locale has the
    following effects:

    \list
    \i If a QLocale object is constructed with the default constructor,
       it will use the default locale's settings.
    \i QString::toInt(), QString::toDouble(), etc., interpret the
       string according to the default locale. If this fails, it
       falls back on the "C" locale.
    \i QString::arg() uses the default locale to format a number when
       its position specifier in the format string contains an 'L',
       e.g. "%L1".
    \endlist

    The following example illustrates how to use QLocale directly:

    \code
        QLocale::setDefault(QLocale::Hebrew, QLocale::Israel);
        QLocale hebrew; // Constructs a default QLocale
        QString s1 = hebrew.toString(15714.3, 'e');

        bool ok;
        double d;

        QLocale::setDefault(QLocale::C);
        d = QString("1234,56").toDouble(&ok);   // ok == false
        d = QString("1234.56").toDouble(&ok);   // ok == true, d == 1234.56

        QLocale::setDefault(QLocale::German);
        d = QString("1234,56").toDouble(&ok);   // ok == true, d == 1234.56
        d = QString("1234.56").toDouble(&ok);   // ok == true, d == 1234.56

        QLocale::setDefault(QLocale::English, QLocale::UnitedStates);
        str = QString("%1 %L2 %L3")
              .arg(12345).arg(12345).arg(12345, 0, 16);
        // str == "12345 12,345 3039"
    \endcode

    When a language/country pair is specified in the constructor, one
    of three things can happen:

    \list
    \i If the language/country pair is found in the database, it is used.
    \i If the language is found but the country is not, or if the country
       is \c AnyCountry, the language is used with the most
       appropriate available country (for example, Germany for German),
    \i If neither the language nor the country are found, QLocale
       defaults to the default locale (see setDefault()).
    \endlist

    The "C" locale is identical to \l{English}/\l{UnitedStates}.

    Use language() and country() to determine the actual language and
    country values used.

    An alternative method for constructing a QLocale object is by
    specifying the locale name.

    \code
        QLocale korean("ko");
        QLocale swiss("de_CH");
    \endcode

    This constructor converts the locale name to a language/country
    pair; it does not use the system locale database.

    The double-to-string and string-to-double conversion functions are
    covered by the following licenses:

    \legalese
    Copyright (c) 1991 by AT&T.

    Permission to use, copy, modify, and distribute this software for any
    purpose without fee is hereby granted, provided that this entire notice
    is included in all copies of any software which is or includes a copy
    or modification of this software and in all copies of the supporting
    documentation for such software.

    THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
    WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
    REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
    OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.

    This product includes software developed by the University of
    California, Berkeley and its contributors.

    \sa QString::arg(), QString::toInt(), QString::toDouble()
*/

/*!
    \enum QLocale::Language

    This enumerated type is used to specify a language.

    \value C
    \value Abkhazian
    \value Afan
    \value Afar
    \value Afrikaans
    \value Albanian
    \value Amharic
    \value Arabic
    \value Armenian
    \value Assamese
    \value Aymara
    \value Azerbaijani
    \value Bashkir
    \value Basque
    \value Bengali
    \value Bhutani
    \value Bihari
    \value Bislama
    \value Bosnian
    \value Breton
    \value Bulgarian
    \value Burmese
    \value Byelorussian
    \value Cambodian
    \value Catalan
    \value Chinese
    \value Cornish
    \value Corsican
    \value Croatian
    \value Czech
    \value Danish
    \value Divehi
    \value Dutch
    \value English
    \value Esperanto
    \value Estonian
    \value Faroese
    \value FijiLanguage
    \value Finnish
    \value French
    \value Frisian
    \value Gaelic
    \value Galician
    \value Georgian
    \value German
    \value Greek
    \value Greenlandic
    \value Guarani
    \value Gujarati
    \value Hausa
    \value Hebrew
    \value Hindi
    \value Hungarian
    \value Icelandic
    \value Indonesian
    \value Interlingua
    \value Interlingue
    \value Inuktitut
    \value Inupiak
    \value Irish
    \value Italian
    \value Japanese
    \value Javanese
    \value Kannada
    \value Kashmiri
    \value Kazakh
    \value Kinyarwanda
    \value Kirghiz
    \value Korean
    \value Kurdish
    \value Kurundi
    \value Laothian
    \value Latin
    \value Latvian
    \value Lingala
    \value Lithuanian
    \value Macedonian
    \value Malagasy
    \value Malay
    \value Malayalam
    \value Maltese
    \value Manx
    \value Maori
    \value Marathi
    \value Moldavian
    \value Mongolian
    \value NauruLanguage
    \value Nepali
    \value Norwegian
    \value Nynorsk
    \value Occitan
    \value Oriya
    \value Pashto
    \value Persian
    \value Polish
    \value Portuguese
    \value Punjabi
    \value Quechua
    \value RhaetoRomance
    \value Romanian
    \value Russian
    \value Samoan
    \value Sangho
    \value Sanskrit
    \value Serbian
    \value SerboCroatian
    \value Sesotho
    \value Setswana
    \value Shona
    \value Sindhi
    \value Singhalese
    \value Siswati
    \value Slovak
    \value Slovenian
    \value Somali
    \value Spanish
    \value Sundanese
    \value Swahili
    \value Swedish
    \value Tagalog
    \value Tajik
    \value Tamil
    \value Tatar
    \value Telugu
    \value Thai
    \value Tibetan
    \value Tigrinya
    \value TongaLanguage
    \value Tsonga
    \value Turkish
    \value Turkmen
    \value Twi
    \value Uigur
    \value Ukrainian
    \value Urdu
    \value Uzbek
    \value Vietnamese
    \value Volapuk
    \value Welsh
    \value Wolof
    \value Xhosa
    \value Yiddish
    \value Yoruba
    \value Zhuang
    \value Zulu
    \omitvalue LastLanguage

    \sa language()
*/

/*!
    \enum QLocale::Country

    This enumerated type is used to specify a country.

    \value AnyCountry
    \value Afghanistan
    \value Albania
    \value Algeria
    \value AmericanSamoa
    \value Andorra
    \value Angola
    \value Anguilla
    \value Antarctica
    \value AntiguaAndBarbuda
    \value Argentina
    \value Armenia
    \value Aruba
    \value Australia
    \value Austria
    \value Azerbaijan
    \value Bahamas
    \value Bahrain
    \value Bangladesh
    \value Barbados
    \value Belarus
    \value Belgium
    \value Belize
    \value Benin
    \value Bermuda
    \value Bhutan
    \value Bolivia
    \value BosniaAndHerzegowina
    \value Botswana
    \value BouvetIsland
    \value Brazil
    \value BritishIndianOceanTerritory
    \value BruneiDarussalam
    \value Bulgaria
    \value BurkinaFaso
    \value Burundi
    \value Cambodia
    \value Cameroon
    \value Canada
    \value CapeVerde
    \value CaymanIslands
    \value CentralAfricanRepublic
    \value Chad
    \value Chile
    \value China
    \value ChristmasIsland
    \value CocosIslands
    \value Colombia
    \value Comoros
    \value DemocraticRepublicOfCongo
    \value PeoplesRepublicOfCongo
    \value CookIslands
    \value CostaRica
    \value IvoryCoast
    \value Croatia
    \value Cuba
    \value Cyprus
    \value CzechRepublic
    \value Denmark
    \value Djibouti
    \value Dominica
    \value DominicanRepublic
    \value EastTimor
    \value Ecuador
    \value Egypt
    \value ElSalvador
    \value EquatorialGuinea
    \value Eritrea
    \value Estonia
    \value Ethiopia
    \value FalklandIslands
    \value FaroeIslands
    \value FijiCountry
    \value Finland
    \value France
    \value MetropolitanFrance
    \value FrenchGuiana
    \value FrenchPolynesia
    \value FrenchSouthernTerritories
    \value Gabon
    \value Gambia
    \value Georgia
    \value Germany
    \value Ghana
    \value Gibraltar
    \value Greece
    \value Greenland
    \value Grenada
    \value Guadeloupe
    \value Guam
    \value Guatemala
    \value Guinea
    \value GuineaBissau
    \value Guyana
    \value Haiti
    \value HeardAndMcDonaldIslands
    \value Honduras
    \value HongKong
    \value Hungary
    \value Iceland
    \value India
    \value Indonesia
    \value Iran
    \value Iraq
    \value Ireland
    \value Israel
    \value Italy
    \value Jamaica
    \value Japan
    \value Jordan
    \value Kazakhstan
    \value Kenya
    \value Kiribati
    \value DemocraticRepublicOfKorea
    \value RepublicOfKorea
    \value Kuwait
    \value Kyrgyzstan
    \value Lao
    \value Latvia
    \value Lebanon
    \value Lesotho
    \value Liberia
    \value LibyanArabJamahiriya
    \value Liechtenstein
    \value Lithuania
    \value Luxembourg
    \value Macau
    \value Macedonia
    \value Madagascar
    \value Malawi
    \value Malaysia
    \value Maldives
    \value Mali
    \value Malta
    \value MarshallIslands
    \value Martinique
    \value Mauritania
    \value Mauritius
    \value Mayotte
    \value Mexico
    \value Micronesia
    \value Moldova
    \value Monaco
    \value Mongolia
    \value Montserrat
    \value Morocco
    \value Mozambique
    \value Myanmar
    \value Namibia
    \value NauruCountry
    \value Nepal
    \value Netherlands
    \value NetherlandsAntilles
    \value NewCaledonia
    \value NewZealand
    \value Nicaragua
    \value Niger
    \value Nigeria
    \value Niue
    \value NorfolkIsland
    \value NorthernMarianaIslands
    \value Norway
    \value Oman
    \value Pakistan
    \value Palau
    \value PalestinianTerritory
    \value Panama
    \value PapuaNewGuinea
    \value Paraguay
    \value Peru
    \value Philippines
    \value Pitcairn
    \value Poland
    \value Portugal
    \value PuertoRico
    \value Qatar
    \value Reunion
    \value Romania
    \value RussianFederation
    \value Rwanda
    \value SaintKittsAndNevis
    \value StLucia
    \value StVincentAndTheGrenadines
    \value Samoa
    \value SanMarino
    \value SaoTomeAndPrincipe
    \value SaudiArabia
    \value Senegal
    \value SerbiaAndMontenegro
    \value Seychelles
    \value SierraLeone
    \value Singapore
    \value Slovakia
    \value Slovenia
    \value SolomonIslands
    \value Somalia
    \value SouthAfrica
    \value SouthGeorgiaAndTheSouthSandwichIslands
    \value Spain
    \value SriLanka
    \value StHelena
    \value StPierreAndMiquelon
    \value Sudan
    \value Suriname
    \value SvalbardAndJanMayenIslands
    \value Swaziland
    \value Sweden
    \value Switzerland
    \value SyrianArabRepublic
    \value Taiwan
    \value Tajikistan
    \value Tanzania
    \value Thailand
    \value Togo
    \value Tokelau
    \value TongaCountry
    \value TrinidadAndTobago
    \value Tunisia
    \value Turkey
    \value Turkmenistan
    \value TurksAndCaicosIslands
    \value Tuvalu
    \value Uganda
    \value Ukraine
    \value UnitedArabEmirates
    \value UnitedKingdom
    \value UnitedStates
    \value UnitedStatesMinorOutlyingIslands
    \value Uruguay
    \value Uzbekistan
    \value Vanuatu
    \value VaticanCityState
    \value Venezuela
    \value VietNam
    \value BritishVirginIslands
    \value USVirginIslands
    \value WallisAndFutunaIslands
    \value WesternSahara
    \value Yemen
    \value Yugoslavia
    \value Zambia
    \value Zimbabwe
    \omitvalue LastCountry

    \sa country()
*/

/*!
    \fn bool QLocale::operator==(const QLocale &other) const

    Returns true if the QLocale object is the same as the \a other
    locale specified; otherwise returns false.
*/

/*!
    \fn bool QLocale::operator!=(const QLocale &other) const

    Returns true if the QLocale object is not the same as the \a other
    locale specified; otherwise returns false.
*/

/*!
    Constructs a QLocale object with the specified \a name,
    which has the format
    "language[_country][.codeset][@modifier]" or "C", where:

    \list
    \i language is a lowercase, two-letter, ISO 639 language code,
    \i territory is an uppercase, two-letter, ISO 3166 country code,
    \i and codeset and modifier are ignored.
    \endlist

    If the string violates the locale format, or language is not
    a valid ISO 369 code, the "C" locale is used instead. If country
    is not present, or is not a valid ISO 3166 code, the most
    appropriate country is chosen for the specified language.

    The language and country codes are converted to their respective
    \c Language and \c Country enums. After this conversion is
    performed the constructor behaves exactly like QLocale(Country,
    Language).

    This constructor is much slower than QLocale(Country, Language).

    \sa name()
*/

QLocale::QLocale(const QString &name)
{
    Language lang = C;
    Country cntry = AnyCountry;

    uint l = name.length();

    do {
        if (l < 2)
            break;

        const QChar *uc = name.unicode();
        if (l > 2
                && uc[2] != QLatin1Char('_')
                && uc[2] != QLatin1Char('.')
                && uc[2] != QLatin1Char('@'))
            break;

        lang = codeToLanguage(name.mid(0, 2));
        if (lang == C)
            break;

        if (l == 2 || uc[2] == QLatin1Char('.') || uc[2] == QLatin1Char('@'))
            break;

        // we have uc[2] == '_'
        if (l < 5)
            break;

        if (l > 5 && uc[5] != QLatin1Char('.') && uc[5] != QLatin1Char('@'))
            break;

        cntry = codeToCountry(name.mid(3, 2));
    } while (false);

    d = findLocale(lang, cntry);
}

/*!
    Constructs a QLocale object initialized with the default locale.

    \sa setDefault()
*/

QLocale::QLocale()
{
    if (default_d == 0)
        default_d = system().d;

    d = default_d;
}

/*!
    Constructs a QLocale object with the specified \a language and \a
    country.

    \list
    \i If the language/country pair is found in the database, it is used.
    \i If the language is found but the country is not, or if the country
       is \c AnyCountry, the language is used with the most
       appropriate available country (for example, Germany for German),
    \i If neither the language nor the country are found, QLocale
       defaults to the default locale (see setDefault()).
    \endlist

    The language and country that are actually used can be queried
    using language() and country().

    \sa setDefault() language() country()
*/

QLocale::QLocale(Language language, Country country)
{
    d = findLocale(language, country);

    // If not found, should default to system
    if (d->languageId() == QLocale::C && language != QLocale::C) {
        if (default_d == 0)
            default_d = system().d;

        d = default_d;
    }
}

/*!
    Constructs a QLocale object as a copy of \a other.
*/

QLocale::QLocale(const QLocale &other)
{
    d = other.d;
}

/*!
    Assigns \a other to this QLocale object and returns a reference
    to this QLocale object.
*/

QLocale &QLocale::operator=(const QLocale &other)
{
    d = other.d;
    return *this;
}

/*!
    \nonreentrant

    Sets the global default locale to \a locale. These
    values are used when a QLocale object is constructed with
    no arguments. If this function is not called, the system's
    locale is used.

    \warning In a multithreaded application, the default locale
    should be set at application startup, before any non-GUI threads
    are created.

    \sa system() c()
*/

void QLocale::setDefault(const QLocale &locale)
{
    default_d = locale.d;
}

/*!
    Returns the language of this locale.

    \sa country(), languageToString(), name()
*/
QLocale::Language QLocale::language() const
{
    return Language(d->languageId());
}

/*!
    Returns the country of this locale.

    \sa language(), countryToString(), name()
*/
QLocale::Country QLocale::country() const
{
    return Country(d->countryId());
}

/*!
    Returns the language and country of this locale as a
    string of the form "language_country", where
    language is a lowercase, two-letter ISO 639 language code,
    and country is an uppercase, two-letter ISO 3166 country code.

    \sa language(), country()
*/

QString QLocale::name() const
{
    Language l = language();

    QString result = languageToCode(l);

    if (l == C)
        return result;

    Country c = country();
    if (c == AnyCountry)
        return result;

    result.append(QLatin1Char('_'));
    result.append(countryToCode(c));

    return result;
}

/*!
    Returns a QString containing the name of \a language.

    \sa countryToString(), name()
*/

QString QLocale::languageToString(Language language)
{
    if (uint(language) > uint(QLocale::LastLanguage))
        return QLatin1String("Unknown");
    return QLatin1String(language_name_list + language_name_index[language]);
}

/*!
    Returns a QString containing the name of \a country.

    \sa country(), name()
*/

QString QLocale::countryToString(Country country)
{
    if (uint(country) > uint(QLocale::LastCountry))
        return QLatin1String("Unknown");
    return QLatin1String(country_name_list + country_name_index[country]);
}

/*!
    Returns the short int represented by the localized string \a s,
    using base \a base. If \a base is 0 the base is determined
    automatically using the following rules: If the string begins with
    "0x", it is assumed to be hexadecimal; if it begins with "0", it
    is assumed to be octal; otherwise it is assumed to be decimal.

    If the conversion fails the function returns 0.

    If \a ok is not 0, failure is reported by setting *ok to false, and
    success by setting *ok to true.

    This function ignores leading and trailing whitespace.

    \sa toUShort(), toString()
*/

short QLocale::toShort(const QString &s, bool *ok, int base) const
{
    qlonglong i = toLongLong(s, ok, base);
    if (i < SHRT_MIN || i > SHRT_MAX) {
        if (ok != 0)
            *ok = false;
        return 0;
    }
    return short(i);
}

/*!
    Returns the unsigned short int represented by the localized string
    \a s, using base \a base. If \a base is 0 the base is determined
    automatically using the following rules: If the string begins with
    "0x", it is assumed to be hexadecimal; if it begins with "0", it
    is assumed to be octal; otherwise it is assumed to be decimal.

    If the conversion fails the function returns 0.

    If \a ok is not 0, failure is reported by setting *ok to false, and
    success by setting *ok to true.

    This function ignores leading and trailing whitespace.

    \sa toShort(), toString()
*/

ushort QLocale::toUShort(const QString &s, bool *ok, int base) const
{
    qulonglong i = toULongLong(s, ok, base);
    if (i > USHRT_MAX) {
        if (ok != 0)
            *ok = false;
        return 0;
    }
    return ushort(i);
}

/*!
    Returns the int represented by the localized string \a s, using
    base \a base. If \a base is 0 the base is determined automatically
    using the following rules: If the string begins with "0x", it is
    assumed to be hexadecimal; if it begins with "0", it is assumed to
    be octal; otherwise it is assumed to be decimal.

    If the conversion fails the function returns 0.

    If \a ok is not 0, failure is reported by setting *ok to false, and
    success by setting *ok to true.

    This function ignores leading and trailing whitespace.

    \sa toUInt(), toString()
*/

int QLocale::toInt(const QString &s, bool *ok, int base) const
{
    qlonglong i = toLongLong(s, ok, base);
    if (i < INT_MIN || i > INT_MAX) {
        if (ok != 0)
            *ok = false;
        return 0;
    }
    return int(i);
}

/*!
    Returns the unsigned int represented by the localized string \a s,
    using base \a base. If \a base is 0 the base is determined
    automatically using the following rules: If the string begins with
    "0x", it is assumed to be hexadecimal; if it begins with "0", it
    is assumed to be octal; otherwise it is assumed to be decimal.

    If the conversion fails the function returns 0.

    If \a ok is not 0, failure is reported by setting *ok to false, and
    success by setting *ok to true.

    This function ignores leading and trailing whitespace.

    \sa toInt(), toString()
*/

uint QLocale::toUInt(const QString &s, bool *ok, int base) const
{
    qulonglong i = toULongLong(s, ok, base);
    if (i > UINT_MAX) {
        if (ok != 0)
            *ok = false;
        return 0;
    }
    return uint(i);
}

/*!
    Returns the long long int represented by the localized string \a
    s, using base \a base. If \a base is 0 the base is determined
    automatically using the following rules: If the string begins with
    "0x", it is assumed to be hexadecimal; if it begins with "0", it
    is assumed to be octal; otherwise it is assumed to be decimal.

    If the conversion fails the function returns 0.

    If \a ok is not 0, failure is reported by setting *ok to false, and
    success by setting *ok to true.

    This function ignores leading and trailing whitespace.

    \sa toInt(), toULongLong(), toDouble(), toString()
*/


qlonglong QLocale::toLongLong(const QString &s, bool *ok, int base) const
{
    return d->stringToLongLong(s, base, ok, QLocalePrivate::ParseGroupSeparators);
}

/*!
    Returns the unsigned long long int represented by the localized
    string \a s, using base \a base. If \a base is 0 the base is
    determined automatically using the following rules: If the string
    begins with "0x", it is assumed to be hexadecimal; if it begins
    with "0", it is assumed to be octal; otherwise it is assumed to be
    decimal.

    If the conversion fails the function returns 0.

    If \a ok is not 0, failure is reported by setting *ok to false, and
    success by setting *ok to true.

    This function ignores leading and trailing whitespace.

    \sa toLongLong(), toInt(), toDouble(), toString()
*/

qlonglong QLocale::toULongLong(const QString &s, bool *ok, int base) const
{
    return d->stringToUnsLongLong(s, base, ok, QLocalePrivate::ParseGroupSeparators);
}

/*!
    Returns the float represented by the localized string \a s, or 0.0
    if the conversion failed.

    If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    This function ignores leading and trailing whitespace.

    \sa toDouble(), toInt(), toString()
*/

#define QT_MAX_FLOAT 3.4028234663852886e+38

float QLocale::toFloat(const QString &s, bool *ok) const
{
    bool myOk;
    double d = toDouble(s, &myOk);
    if (!myOk || d > QT_MAX_FLOAT || d < -QT_MAX_FLOAT) {
        if (ok != 0)
            *ok = false;
        return 0.0;
    }
    if (ok != 0)
        *ok = true;
    return float(d);
}

/*!
    Returns the double represented by the localized string \a s, or
    0.0 if the conversion failed.

    If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    Unlike QString::toDouble(), this function does not fall back to
    the "C" locale if the string cannot be interpreted in this
    locale.

    \code
        bool ok;
        double d;

        QLocale c(QLocale::C);
        d = c.toDouble( "1234.56", &ok );  // ok == true, d == 1234.56
        d = c.toDouble( "1,234.56", &ok ); // ok == true, d == 1234.56
        d = c.toDouble( "1234,56", &ok );  // ok == false

        QLocale german(QLocale::German);
        d = german.toDouble( "1234,56", &ok );  // ok == true, d == 1234.56
        d = german.toDouble( "1.234,56", &ok ); // ok == true, d == 1234.56
        d = german.toDouble( "1234.56", &ok );  // ok == false

        d = german.toDouble( "1.234", &ok );    // ok == true, d == 1234.0
    \endcode

    Notice that the last conversion returns 1234.0, because '.' is the
    thousands group separator in the German locale.

    This function ignores leading and trailing whitespace.

    \sa toFloat(), toInt(), toString()
*/

double QLocale::toDouble(const QString &s, bool *ok) const
{
    return d->stringToDouble(s, ok, QLocalePrivate::ParseGroupSeparators);
}

/*!
    Returns a localized string representation of \a i.

    \sa toLongLong()
*/

QString QLocale::toString(qlonglong i) const
{
    return d->longLongToString(i, -1, 10, -1, QLocalePrivate::ThousandsGroup);
}

/*!
    \overload

    \sa toULongLong()
*/

QString QLocale::toString(qulonglong i) const
{
    return d->unsLongLongToString(i, -1, 10, -1, QLocalePrivate::ThousandsGroup);
}

static int repeatCount(const QString &s, int i)
{
    QChar c = s.at(i);
    int j = i + 1;
    while (j < s.size() && s.at(j) == c)
        ++j;
    return j - i;
}

static QString readEscapedFormatString(const QString &format, int *idx)
{
    int &i = *idx;

    Q_ASSERT(format.at(i++).unicode() == '\'');
    if (i == format.size())
        return QString();
    if (format.at(i).unicode() == '\'') { // "''" outside of a quoted stirng
        ++i;
        return QLatin1String("'");
    }

    QString result;

    while (i < format.size()) {
        if (format.at(i).unicode() == '\'') {
            if (i + 1 < format.size() && format.at(i + 1).unicode() == '\'') {
                // "''" inside of a quoted string
                result.append(QLatin1Char('\''));
                i += 2;
            } else {
                break;
            }
        } else {
            result.append(format.at(i++));
        }
    }
    if (i < format.size())
        ++i;

    return result;
}

/*!
    Returns a localized string representation of \a date. If \a format is specified,
    the string is formatted according to it. If \a format is an empty strig (the
    default value), something happens, but I'm not certain what it is.
*/

QString QLocale::toString(const QDate &date, const QString &format) const
{
    QString result;

    int i = 0;
    while (i < format.size()) {
        if (format.at(i).unicode() == '\'') {
            result.append(readEscapedFormatString(format, &i));
            continue;
        }

        QChar c = format.at(i);
        int repeat = repeatCount(format, i);

        switch (c.unicode()) {
            case 'y':
                if (repeat >= 4)
                    repeat = 4;
                else if (repeat >= 2)
                    repeat = 2;
                else
                    repeat = 1;

                switch (repeat) {
                    case 4:
                        result.append(d->longLongToString(date.year()));
                        break;
                    case 2:
                        result.append(d->longLongToString(date.year()%100));
                        break;
                    default:
                        result.append(c);
                        break;
                }
                break;

            case 'M':
                if (repeat > 4)
                    repeat = 4;
                switch (repeat) {
                    case 1:
                        result.append(d->longLongToString(date.month()));
                        break;
                    case 2:
                        result.append(d->longLongToString(date.month(), -1, 10, 2, QLocalePrivate::ZeroPadded));
                        break;
                    case 3:
                        result.append(d->month(date.month() - 1, true));
                        break;
                    case 4:
                        result.append(d->month(date.month() - 1));
                        break;
                    default:
                        break;
                }
                break;

            case 'd':
                if (repeat > 4)
                    repeat = 4;
                switch (repeat) {
                    case 1:
                        result.append(d->longLongToString(date.day()));
                        break;
                    case 2:
                        result.append(d->longLongToString(date.day(), -1, 10, 2, QLocalePrivate::ZeroPadded));
                        break;
                    case 3:
                        result.append(d->day(date.dayOfWeek() - 1, true));
                        break;
                    case 4:
                        result.append(d->day(date.dayOfWeek() - 1));
                        break;
                    default:
                        break;
                }
                break;

            default:
                repeat = 1;
                result.append(c);
                break;
        }

        i += repeat;
    }

    return result;
}

QString QLocale::toString(const QDate &date, FormatType format) const
{
    QString format_str = dateFormat(format);
    return toString(date, format_str);
}

static bool timeFormatContainsAP(const QString &format)
{
    int i = 0;
    while (i + 1 < format.size()) {
        if (format.at(i).unicode() == '\'') {
            readEscapedFormatString(format, &i);
            continue;
        }

        QChar c1 = format.at(i);
        QChar c2 = format.at(i + 1);
        if (c1.unicode() == 'a' && c2.unicode() == 'p'
            || c1.unicode() == 'A' && c2.unicode() == 'P')
            return true;

        ++i;
    }
    return false;
}

QString QLocale::toString(const QTime &time, const QString &format) const
{
    QString result;

    int hour = time.hour();
    enum { AM, PM } am_pm = AM;
    if (timeFormatContainsAP(format)) {
        // you can't go wrong this way :)
        if (hour == 0) {
            am_pm = AM;
            hour = 12;
        } else if (hour < 12) {
            am_pm = AM;
        } else if (hour == 12) {
            am_pm = PM;
        } else {
            am_pm = PM;
            hour -= 12;
        }
    }

    int i = 0;

    while (i < format.size()) {
        if (format.at(i).unicode() == '\'') {
            result.append(readEscapedFormatString(format, &i));
            continue;
        }

        QChar c = format.at(i);
        int repeat = repeatCount(format, i);

        switch (c.unicode()) {
            case 'h':
                if (repeat > 2)
                    repeat = 2;

                switch (repeat) {
                    case 1:
                        result.append(d->longLongToString(hour));
                        break;
                    case 2:
                        result.append(d->longLongToString(hour, -1, 10, 2, QLocalePrivate::ZeroPadded));
                        break;
                    default:
                        break;
                }
                break;

            case 'm':
                if (repeat > 2)
                    repeat = 2;
                switch (repeat) {
                    case 1:
                        result.append(d->longLongToString(time.minute()));
                        break;
                    case 2:
                        result.append(d->longLongToString(time.minute(), -1, 10, 2, QLocalePrivate::ZeroPadded));
                        break;
                    default:
                        break;
                }
                break;

            case 's':
                if (repeat > 2)
                    repeat = 2;
                switch (repeat) {
                    case 1:
                        result.append(d->longLongToString(time.second()));
                        break;
                    case 2:
                        result.append(d->longLongToString(time.second(), -1, 10, 2, QLocalePrivate::ZeroPadded));
                        break;
                    default:
                        break;
                }
                break;

            case 'a':
                if (i + 1 < format.length() && format.at(i + 1).unicode() == 'p') {
                    repeat = 2;
                    result.append(am_pm == AM ? QLatin1String("am") : QLatin1String("pm"));
                } else {
                    repeat = 1;
                    result.append(c);
                }
                break;

            case 'A':
                if (i + 1 < format.length() && format.at(i + 1).unicode() == 'P') {
                    repeat = 2;
                    result.append(am_pm == AM ? QLatin1String("AM") : QLatin1String("PM"));
                } else {
                    repeat = 1;
                    result.append(c);
                }
                break;

            default:
                repeat = 1;
                result.append(c);
                break;
        }

        i += repeat;
    }

    return result;
}

QString QLocale::toString(const QTime &time, FormatType format) const
{
    QString format_str = timeFormat(format);
    return toString(time, format_str);
}

/*!
    \since 4.1

    Returns the date format used for the current locale.

    If \a format is ShortFormat the format will be a short version.
    Otherwise it uses a longer version.

    \sa QDate::toString(), QDate::fromString()
*/

QString QLocale::dateFormat(FormatType format) const
{
#ifdef Q_OS_MAC
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if(d == system().d) {
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
            QCFType<CFLocaleRef> l = CFLocaleCopyCurrent();
            CFDateFormatterStyle f = kCFDateFormatterLongStyle;
            if(format == ShortFormat)
                f = kCFDateFormatterShortStyle;
            QCFType<CFDateFormatterRef> formatter = CFDateFormatterCreate(kCFAllocatorDefault,
                                                                          l, f, kCFDateFormatterNoStyle);
            CFStringRef cfstr = CFDateFormatterGetFormat(formatter);
            QString ret(QCFString::toQString(cfstr));
            if(!ret.isEmpty())
                return ret;
        }
    }
#endif
#endif
    const quint32 idx = (format == ShortFormat ? d->m_short_date_format_idx : d->m_long_date_format_idx);
    return QString::fromUtf8(date_format_data + idx);
}

/*!
    \since 4.1

    Returns the time format used for the current locale.

    If \a format is ShortFormat the format will be a short version.
    Otherwise it uses a longer version.

    \sa QTime::toString(), QTime::fromString()
*/

QString QLocale::timeFormat(FormatType format) const
{
#ifdef Q_OS_MAC
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if(d == system().d) {
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
            QCFType<CFLocaleRef> l = CFLocaleCopyCurrent();
            CFDateFormatterStyle f = kCFDateFormatterLongStyle;
            if(format == ShortFormat)
                f = kCFDateFormatterShortStyle;
            QCFType<CFDateFormatterRef> formatter = CFDateFormatterCreate(kCFAllocatorDefault,
                                                                          l, kCFDateFormatterNoStyle, f);
            CFStringRef cfstr = CFDateFormatterGetFormat(formatter);
            QString ret(QCFString::toQString(cfstr));
            if(!ret.isEmpty())
                return ret;
        }
    }
#endif
#endif
    const quint32 idx = (format == ShortFormat ? d->m_short_time_format_idx : d->m_long_time_format_idx);
    return QString::fromUtf8(time_format_data + idx);
}

/*!
    \since 4.1

    Returns the decimal point character of this locale.
*/
QChar QLocale::decimalPoint() const
{
    return d->decimal();
}

/*!
    \since 4.1

    Returns the group separator character of this locale.
*/
QChar QLocale::groupSeparator() const
{
    return d->group();
}

/*!
    \since 4.1

    Returns the percent character of this locale.
*/
QChar QLocale::percent() const
{
    return d->percent();
}

/*!
    \since 4.1

    Returns the zero digit character of this locale.
*/
QChar QLocale::zeroDigit() const
{
    return d->zero();
}

/*!
    \since 4.1

    Returns the negative sign character of this locale.
*/
QChar QLocale::negativeSign() const
{
    return d->minus();
}

/*!
    \since 4.1

    Returns the exponential character of this locale.
*/
QChar QLocale::exponential() const
{
    return d->exponential();
}

static bool qIsUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static char qToLower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    else
        return c;
}

/*!
    \overload

    \a f and \a prec have the same meaning as in QString::number(double, char, int).

    \sa toDouble()
*/

QString QLocale::toString(double i, char f, int prec) const
{
    QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
    uint flags = 0;

    if (qIsUpper(f))
        flags = QLocalePrivate::CapitalEorX;
    f = qToLower(f);

    switch (f) {
        case 'f':
            form = QLocalePrivate::DFDecimal;
            break;
        case 'e':
            form = QLocalePrivate::DFExponent;
            break;
        case 'g':
            form = QLocalePrivate::DFSignificantDigits;
            break;
        default:
            break;
    }

    flags |= QLocalePrivate::ThousandsGroup;
    return d->doubleToString(i, prec, form, -1, flags);
}

/*!
    \fn QLocale QLocale::c()

    Returns a QLocale object initialized to the "C" locale.

    \sa system()
*/

/*!
    Returns a QLocale object initialized to the system locale.

    \sa QTextCodec::locale() c()
*/

QLocale QLocale::system()
{
    QByteArray s = 0;
#ifdef Q_OS_UNIX
    s = qgetenv("LC_ALL");
    if (s.isNull())
        s = qgetenv("LC_NUMERIC");
    if (s.isNull())
#endif
        s = QLocalePrivate::systemLocaleName();
    return QLocale(QString::fromLocal8Bit(s));
}

/*!
\fn QString QLocale::toString(short i) const

\overload

\sa toShort()
*/

/*!
\fn QString QLocale::toString(ushort i) const

\overload

\sa toUShort()
*/

/*!
\fn QString QLocale::toString(int i) const

\overload

\sa toInt()
*/

/*!
\fn QString QLocale::toString(uint i) const

\overload

\sa toUInt()
*/

/*
\fn QString QLocale::toString(long i) const

\overload

\sa  toLong()
*/

/*
\fn QString QLocale::toString(ulong i) const

\overload

\sa toULong()
*/

/*!
\fn QString QLocale::toString(float i, char f = 'g', int prec = 6) const

\overload

\a f and \a prec have the same meaning as in QString::number(double, char, int).

\sa toDouble()
*/


static QString qulltoa(qulonglong l, int base, const QLocalePrivate &locale)
{
    QChar buff[65]; // length of MAX_ULLONG in base 2
    QChar *p = buff + 65;

    if (base != 10 || locale.zero().unicode() == '0') {
        while (l != 0) {
            int c = l % base;

            --p;

            if (c < 10)
                *p = '0' + c;
            else
                *p = c - 10 + 'a';

            l /= base;
        }
    }
    else {
        while (l != 0) {
            int c = l % base;

            *(--p) = locale.zero().unicode() + c;

            l /= base;
        }
    }

    return QString(p, 65 - (p - buff));
}

static QString qlltoa(qlonglong l, int base, const QLocalePrivate &locale)
{
    return qulltoa(l < 0 ? -l : l, base, locale);
}

enum PrecisionMode {
    PMDecimalDigits =             0x01,
    PMSignificantDigits =   0x02,
    PMChopTrailingZeros =   0x03
};

static QString &decimalForm(QString &digits, int decpt, uint precision,
                            PrecisionMode pm,
                            bool always_show_decpt,
                            bool thousands_group,
                            const QLocalePrivate &locale)
{
    if (decpt < 0) {
        for (int i = 0; i < -decpt; ++i)
            digits.prepend(locale.zero());
        decpt = 0;
    }
    else if (decpt > digits.length()) {
        for (int i = digits.length(); i < decpt; ++i)
            digits.append(locale.zero());
    }

    if (pm == PMDecimalDigits) {
        uint decimal_digits = digits.length() - decpt;
        for (uint i = decimal_digits; i < precision; ++i)
            digits.append(locale.zero());
    }
    else if (pm == PMSignificantDigits) {
        for (uint i = digits.length(); i < precision; ++i)
            digits.append(locale.zero());
    }
    else { // pm == PMChopTrailingZeros
    }

    if (always_show_decpt || decpt < digits.length())
        digits.insert(decpt, locale.decimal());

    if (thousands_group) {
        for (int i = decpt - 3; i > 0; i -= 3)
            digits.insert(i, locale.group());
    }

    if (decpt == 0)
        digits.prepend(locale.zero());

    return digits;
}

static QString &exponentForm(QString &digits, int decpt, uint precision,
                                PrecisionMode pm,
                                bool always_show_decpt,
                                const QLocalePrivate &locale)
{
    int exp = decpt - 1;

    if (pm == PMDecimalDigits) {
        for (uint i = digits.length(); i < precision + 1; ++i)
            digits.append(locale.zero());
    }
    else if (pm == PMSignificantDigits) {
        for (uint i = digits.length(); i < precision; ++i)
            digits.append(locale.zero());
    }
    else { // pm == PMChopTrailingZeros
    }

    if (always_show_decpt || digits.length() > 1)
        digits.insert(1, locale.decimal());

    digits.append(locale.exponential());
    digits.append(locale.longLongToString(exp, 2, 10,
                    -1, QLocalePrivate::AlwaysShowSign));

    return digits;
}

static bool isZero(double d)
{
    uchar *ch = (uchar *)&d;
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return !(ch[0] & 0x7F || ch[1] || ch[2] || ch[3] || ch[4] || ch[5] || ch[6] || ch[7]);
    } else {
        return !(ch[7] & 0x7F || ch[6] || ch[5] || ch[4] || ch[3] || ch[2] || ch[1] || ch[0]);
    }
}

QString QLocalePrivate::doubleToString(double d,
                                        int precision,
                                        DoubleForm form,
                                        int width,
                                        unsigned flags) const
{
    if (precision == -1)
        precision = 6;
    if (width == -1)
        width = 0;

    bool negative = false;
    bool special_number = false; // nan, +/-inf
    QString num_str;

    // Detect special numbers (nan, +/-inf)
    if (qIsInf(d)) {
        num_str = infinity();
        special_number = true;
        negative = d < 0;
    } else if (qIsNan(d)) {
        num_str = nan();
        special_number = true;
    }

    // Handle normal numbers
    if (!special_number) {
        int decpt, sign;
        QString digits;

#ifdef QT_QLOCALE_USES_FCVT
        // NOT thread safe!
        if (form == DFDecimal) {
            digits = QLatin1String(fcvt(d, precision, &decpt, &sign));
        } else {
            int pr = precision;
            if (form == DFExponent)
                ++pr;
            else if (form == DFSignificantDigits && pr == 0)
                pr = 1;
            digits = QLatin1String(ecvt(d, pr, &decpt, &sign));

            // Chop trailing zeros
            if (digits.length() > 0) {
                int last_nonzero_idx = digits.length() - 1;
                while (last_nonzero_idx > 0
                       && digits.unicode()[last_nonzero_idx] == '0')
                    --last_nonzero_idx;
                digits.truncate(last_nonzero_idx + 1);
            }

        }

#else
        int mode;
        if (form == DFDecimal)
            mode = 3;
        else
            mode = 2;

        /* This next bit is a bit quirky. In DFExponent form, the precision
           is the number of digits after decpt. So that would suggest using
           mode=3 for qdtoa. But qdtoa behaves strangely when mode=3 and
           precision=0. So we get around this by using mode=2 and reasoning
           that we want precision+1 significant digits, since the decimal
           point in this mode is always after the first digit. */
        int pr = precision;
        if (form == DFExponent)
            ++pr;

        char *rve = 0;
        char *buff = 0;
        digits = QLatin1String(qdtoa(d, mode, pr, &decpt, &sign, &rve, &buff));
        if (buff != 0)
            free(buff);
#endif // QT_QLOCALE_USES_FCVT

        if (zero().unicode() != '0') {
            ushort z = zero().unicode() - '0';
            for (int i = 0; i < digits.length(); ++i)
                reinterpret_cast<ushort *>(digits.data())[i] += z;
        }

        bool always_show_decpt = flags & Alternate;
        switch (form) {
            case DFExponent: {
                num_str = exponentForm(digits, decpt, precision, PMDecimalDigits,
                                                    always_show_decpt, *this);
                break;
            }
            case DFDecimal: {
                num_str = decimalForm(digits, decpt, precision, PMDecimalDigits,
                                        always_show_decpt, flags & ThousandsGroup,
                                        *this);
                break;
            }
            case DFSignificantDigits: {
                PrecisionMode mode = (flags & Alternate) ?
                            PMSignificantDigits : PMChopTrailingZeros;

                if (decpt != digits.length() && (decpt <= -4 || decpt > precision))
                    num_str = exponentForm(digits, decpt, precision, mode,
                                                    always_show_decpt, *this);
                else
                    num_str = decimalForm(digits, decpt, precision, mode,
                                            always_show_decpt, flags & ThousandsGroup,
                                            *this);
                break;
            }
        }

        negative = sign != 0 && !isZero(d);
    }

    // pad with zeros. LeftAdjusted overrides this flag). Also, we don't
    // pad special numbers
    if (flags & QLocalePrivate::ZeroPadded
            && !(flags & QLocalePrivate::LeftAdjusted)
            && !special_number) {
        int num_pad_chars = width - num_str.length();
        // leave space for the sign
        if (negative
                || flags & QLocalePrivate::AlwaysShowSign
                || flags & QLocalePrivate::BlankBeforePositive)
            --num_pad_chars;

        for (int i = 0; i < num_pad_chars; ++i)
            num_str.prepend(zero());
    }

    // add sign
    if (negative)
        num_str.prepend(minus());
    else if (flags & QLocalePrivate::AlwaysShowSign)
        num_str.prepend(plus());
    else if (flags & QLocalePrivate::BlankBeforePositive)
        num_str.prepend(QLatin1Char(' '));

    if (flags & QLocalePrivate::CapitalEorX)
        num_str = num_str.toUpper();

    return num_str;
}

QString QLocalePrivate::longLongToString(qlonglong l, int precision,
                                            int base, int width,
                                            unsigned flags) const
{
    bool precision_not_specified = false;
    if (precision == -1) {
        precision_not_specified = true;
        precision = 1;
    }

    bool negative = l < 0;
    if (base != 10) {
        // these are not suported by sprintf for octal and hex
        flags &= ~AlwaysShowSign;
        flags &= ~BlankBeforePositive;
        negative = false; // neither are negative numbers
    }

    QString num_str;
    if (base == 10)
        num_str = qlltoa(l, base, *this);
    else
        num_str = qulltoa(l, base, *this);

    uint cnt_thousand_sep = 0;
    if (flags & ThousandsGroup && base == 10) {
        for (int i = num_str.length() - 3; i > 0; i -= 3) {
            num_str.insert(i, group());
            ++cnt_thousand_sep;
        }
    }

    for (int i = num_str.length()/* - cnt_thousand_sep*/; i < precision; ++i)
        num_str.prepend(base == 10 ? zero() : QChar::fromLatin1('0'));

    if (flags & Alternate
            && base == 8
            && (num_str.isEmpty() || num_str[0].unicode() != QLatin1Char('0')))
        num_str.prepend(QLatin1Char('0'));

    // LeftAdjusted overrides this flag ZeroPadded. sprintf only padds
    // when precision is not specified in the format string
    bool zero_padded = flags & ZeroPadded
                        && !(flags & LeftAdjusted)
                        && precision_not_specified;

    if (zero_padded) {
        int num_pad_chars = width - num_str.length();

        // leave space for the sign
        if (negative
                || flags & AlwaysShowSign
                || flags & BlankBeforePositive)
            --num_pad_chars;

        // leave space for optional '0x' in hex form
        if (base == 16
                && flags & Alternate
                && l != 0)
            num_pad_chars -= 2;

        for (int i = 0; i < num_pad_chars; ++i)
            num_str.prepend(base == 10 ? zero() : QChar::fromLatin1('0'));
    }

    if (base == 16
            && flags & Alternate
            && l != 0)
        num_str.prepend(QLatin1String("0x"));

    // add sign
    if (negative)
        num_str.prepend(minus());
    else if (flags & AlwaysShowSign)
        num_str.prepend(base == 10 ? plus() : QChar::fromLatin1('+'));
    else if (flags & BlankBeforePositive)
        num_str.prepend(QLatin1Char(' '));

    if (flags & CapitalEorX)
        num_str = num_str.toUpper();

    return num_str;
}

QString QLocalePrivate::unsLongLongToString(qulonglong l, int precision,
                                            int base, int width,
                                            unsigned flags) const
{
    bool precision_not_specified = false;
    if (precision == -1) {
        precision_not_specified = true;
        precision = 1;
    }

    QString num_str = qulltoa(l, base, *this);

    uint cnt_thousand_sep = 0;
    if (flags & ThousandsGroup && base == 10) {
        for (int i = num_str.length() - 3; i > 0; i -=3) {
            num_str.insert(i, group());
            ++cnt_thousand_sep;
        }
    }

    for (int i = num_str.length()/* - cnt_thousand_sep*/; i < precision; ++i)
        num_str.prepend(base == 10 ? zero() : QChar::fromLatin1('0'));

    if (flags & Alternate
            && base == 8
            && (num_str.isEmpty() || num_str[0].unicode() != QLatin1Char('0')))
        num_str.prepend(QLatin1Char('0'));

    // LeftAdjusted overrides this flag ZeroPadded. sprintf only padds
    // when precision is not specified in the format string
    bool zero_padded = flags & ZeroPadded
                        && !(flags & LeftAdjusted)
                        && precision_not_specified;

    if (zero_padded) {
        int num_pad_chars = width - num_str.length();

        // leave space for optional '0x' in hex form
        if (base == 16
                && flags & Alternate
                && l != 0)
            num_pad_chars -= 2;

        for (int i = 0; i < num_pad_chars; ++i)
            num_str.prepend(base == 10 ? zero() : QChar::fromLatin1('0'));
    }

    if (base == 16
            && flags & Alternate
            && l != 0)
        num_str.prepend(QLatin1String("0x"));

    if (flags & CapitalEorX)
        num_str = num_str.toUpper();

    return num_str;
}

// Removes thousand-group separators in "C" locale.
static bool removeGroupSeparators(QLocalePrivate::CharBuff *num)
{
    int group_cnt = 0; // counts number of group chars
    int decpt_idx = -1;

    char *data = num->data();
    int l = qstrlen(data);

    // Find the decimal point and check if there are any group chars
    int i = 0;
    for (; i < l; ++i) {
        char c = data[i];

        if (c == ',') {
            if (i == 0 || data[i - 1] < '0' || data[i - 1] > '9')
                return false;
            if (i == l || data[i + 1] < '0' || data[i + 1] > '9')
                return false;
            ++group_cnt;
        }
        else if (c == '.') {
            // Fail if more than one decimal points
            if (decpt_idx != -1)
                return false;
            decpt_idx = i;
        } else if (c == 'e' || c == 'E') {
            // an 'e' or 'E' - if we have not encountered a decimal
            // point, this is where it "is".
            if (decpt_idx == -1)
                decpt_idx = i;
        }
    }

    // If no group chars, we're done
    if (group_cnt == 0)
        return true;

    // No decimal point means that it "is" at the end of the string
    if (decpt_idx == -1)
        decpt_idx = l;

    i = 0;
    while (i < l && group_cnt > 0) {
        char c = data[i];

        if (c == ',') {
            // Don't allow group chars after the decimal point
            if (i > decpt_idx)
                return false;

            // Check that it is placed correctly relative to the decpt
            if ((decpt_idx - i) % 4 != 0)
                return false;

            // Remove it
            memmove(data + i, data + i + 1, l - i - 1);
            data[--l] = '\0';

            --group_cnt;
            --decpt_idx;
        } else {
            // Check that we are not missing a separator
            if (i < decpt_idx && (decpt_idx - i) % 4 == 0)
                return false;
            ++i;
        }
    }

    return true;
}

/*
    Converts a number in locale to its representation in the C locale.
    Only has to guarantee that a string that is a correct representation of
    a number will be converted. If junk is passed in, junk will be passed
    out and the error will be detected during the actual conversion to a
    number. We can't detect junk here, since we don't even know the base
    of the number.
*/
bool QLocalePrivate::numberToCLocale(const QString &num,
                                            GroupSeparatorMode group_sep_mode,
                                            CharBuff *result) const
{
    const QChar *uc = num.unicode();
    int l = num.length();
    int idx = 0;
    const ushort zeroUnicode = zero().unicode();
    const ushort tenUnicode = zeroUnicode + 10;

    // Skip whitespace
    while (idx < l && uc[idx].isSpace())
        ++idx;
    if (idx == l)
        return false;

    while (idx < l) {
        char out;
        const QChar &in = uc[idx];

        if (in.unicode() >= zeroUnicode && in.unicode() < tenUnicode)
            out = '0' + in.unicode() - zeroUnicode;
        else if (in == plus())
            out = '+';
        else if (in == minus())
            out = '-';
        else if (in == decimal())
            out = '.';
        else if (in == group())
            out = ',';
        // In several languages group() is the char 0xA0, which looks like a space.
        // People use a regular space instead of it and complain it doesn't work.
        else if (group().unicode() == 0xA0 && in.unicode() == ' ')
            out = ',';
        else if (in == exponential() || in == exponential().toUpper())
            out = 'e';
        else if (in == list())
            out = ';';
        else if (in == percent())
            out = '%';
        else if (in.unicode() >= 'A' && in.unicode() <= 'Z')
            out = in.toLower().toLatin1();
        else if (in.unicode() >= 'a' && in.unicode() <= 'z')
            out = in.toLatin1();
        else
            break;

        result->append(out);

        ++idx;
    }

    // Check trailing whitespace
    for (; idx < l; ++idx) {
        if (!uc[idx].isSpace())
            return false;
    }

    result->append('\0');

    // Check separators
    if (group_sep_mode == ParseGroupSeparators
            && !removeGroupSeparators(result))
        return false;


    return true;
}

double QLocalePrivate::stringToDouble(const QString &number, bool *ok,
                                        GroupSeparatorMode group_sep_mode) const
{
    CharBuff buff;
    if (!numberToCLocale(number, group_sep_mode, &buff)) {
        if (ok != 0)
            *ok = false;
        return 0.0;
    }

    return bytearrayToDouble(buff.constData(), ok);
}

qlonglong QLocalePrivate::stringToLongLong(const QString &number, int base,
                                           bool *ok, GroupSeparatorMode group_sep_mode) const
{
    CharBuff buff;
    if (!numberToCLocale(number, group_sep_mode, &buff)) {
        if (ok != 0)
            *ok = false;
        return 0;
    }

    return bytearrayToLongLong(buff.constData(), base, ok);
}

qulonglong QLocalePrivate::stringToUnsLongLong(const QString &number, int base,
                                               bool *ok, GroupSeparatorMode group_sep_mode) const
{
    CharBuff buff;
    if (!numberToCLocale(number, group_sep_mode, &buff)) {
        if (ok != 0)
            *ok = false;
        return 0;
    }

    return bytearrayToUnsLongLong(buff.constData(), base, ok);
}


double QLocalePrivate::bytearrayToDouble(const char *num, bool *ok)
{
    if (ok != 0)
        *ok = true;

    if (qstrcmp(num, "nan") == 0)
        return Q_SNAN;

    if (qstrcmp(num, "+inf") == 0 || qstrcmp(num, "inf") == 0)
        return Q_INFINITY;

    if (qstrcmp(num, "-inf") == 0)
        return -Q_INFINITY;

    bool _ok;
#ifdef QT_QLOCALE_USES_FCVT
    char *endptr;
    double d = strtod(num, &endptr);
    _ok = true;
#else
    const char *endptr;
    double d = qstrtod(num, &endptr, &_ok);
#endif

    if (!_ok || *endptr != '\0') {
        if (ok != 0)
            *ok = false;
        return 0.0;
    }
    else
        return d;
}

qlonglong QLocalePrivate::bytearrayToLongLong(const char *num, int base, bool *ok)
{
    bool _ok;
    const char *endptr;
    qlonglong l = qstrtoll(num, &endptr, base, &_ok);

    if (!_ok || *endptr != '\0') {
        if (ok != 0)
            *ok = false;
        return 0;
    }

    if (ok != 0)
        *ok = true;
    return l;
}

qulonglong QLocalePrivate::bytearrayToUnsLongLong(const char *num, int base, bool *ok)
{
    bool _ok;
    const char *endptr;
    qulonglong l = qstrtoull(num, &endptr, base, &_ok);

    if (!_ok || *endptr != '\0') {
        if (ok != 0)
            *ok = false;
        return 0;
    }

    if (ok != 0)
        *ok = true;
    return l;
}

/*-
 * Copyright (c) 1992, 1993
 *        The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the University of
 *        California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// static char sccsid[] = "@(#)strtouq.c        8.1 (Berkeley) 6/4/93";
//  "$FreeBSD: src/lib/libc/stdlib/strtoull.c,v 1.5.2.1 2001/03/02 09:45:20 obrien Exp $";

/*
 * Convert a string to an unsigned long long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
static qulonglong qstrtoull(const char *nptr, const char **endptr, register int base, bool *ok)
{
    register const char *s = nptr;
    register qulonglong acc;
    register unsigned char c;
    register qulonglong qbase, cutoff;
    register int neg, any, cutlim;

    if (ok != 0)
        *ok = true;

    /*
     * See strtoq for comments as to the logic used.
     */
    s = nptr;
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        if (ok != 0)
            *ok = false;
        if (endptr != 0)
            *endptr = s - 1;
        return 0;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    qbase = unsigned(base);
    cutoff = qulonglong(ULLONG_MAX) / qbase;
    cutlim = qulonglong(ULLONG_MAX) % qbase;
    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c))
            break;
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= qbase;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULLONG_MAX;
        if (ok != 0)
            *ok = false;
    }
    else if (neg)
        acc = (~acc) + 1;
    if (endptr != 0)
        *endptr = (any ? s - 1 : nptr);
    return acc;
}


//  "$FreeBSD: src/lib/libc/stdlib/strtoll.c,v 1.5.2.1 2001/03/02 09:45:20 obrien Exp $";


/*
 * Convert a string to a long long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
static qlonglong qstrtoll(const char *nptr, const char **endptr, register int base, bool *ok)
{
    register const char *s;
    register qulonglong acc;
    register unsigned char c;
    register qulonglong qbase, cutoff;
    register int neg, any, cutlim;

    if (ok != 0)
        *ok = true;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    s = nptr;
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for quads is
     * [-9223372036854775808..9223372036854775807] and the input base
     * is 10, cutoff will be set to 922337203685477580 and cutlim to
     * either 7 (neg==0) or 8 (neg==1), meaning that if we have
     * accumulated a value > 922337203685477580, or equal but the
     * next digit is > 7 (or 8), the number is too big, and we will
     * return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    qbase = unsigned(base);
    cutoff = neg ? qulonglong(0-(LLONG_MIN + LLONG_MAX)) + LLONG_MAX : LLONG_MAX;
    cutlim = cutoff % qbase;
    cutoff /= qbase;
    for (acc = 0, any = 0;; c = *s++) {
        if (!isascii(c))
            break;
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= qbase;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LLONG_MIN : LLONG_MAX;
        if (ok != 0)
            *ok = false;
    } else if (neg) {
        acc = (~acc) + 1;
    }
    if (endptr != 0)
        *endptr = (any ? s - 1 : nptr);
    return acc;
}

#ifndef QT_QLOCALE_USES_FCVT

/*        From: NetBSD: strtod.c,v 1.26 1998/02/03 18:44:21 perry Exp */
/* $FreeBSD: src/lib/libc/stdlib/netbsd_strtod.c,v 1.2.2.2 2001/03/02 17:14:15 tegge Exp $        */

/* Please send bug reports to
        David M. Gay
        AT&T Bell Laboratories, Room 2C-463
        600 Mountain Avenue
        Murray Hill, NJ 07974-2070
        U.S.A.
        dmg@research.att.com or research!dmg
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *        1. We only require IEEE, IBM, or VAX double-precision
 *                arithmetic (not IEEE double-extended).
 *        2. We get by with floating-point arithmetic in a case that
 *                Clinger missed -- when we're computing d * 10^n
 *                for a small integer d and the integer n is not too
 *                much larger than 22 (the maximum integer k for which
 *                we can represent 10^k exactly), we may be able to
 *                compute (d*10^k) * 10^(e-k) with just one roundoff.
 *        3. Rather than a bit-at-a-time adjustment of the binary
 *                result in the hard case, we use floating-point
 *                arithmetic to determine the adjustment to within
 *                one bit; only in really hard cases do we need to
 *                compute a second residual.
 *        4. Because of 3., we don't need a large table of powers of 10
 *                for ten-to-e (just some small tables, e.g. of 10^k
 *                for 0 <= k <= 22).
 */

/*
 * #define IEEE_LITTLE_ENDIAN for IEEE-arithmetic machines where the least
 *        significant byte has the lowest address.
 * #define IEEE_BIG_ENDIAN for IEEE-arithmetic machines where the most
 *        significant byte has the lowest address.
 * #define Long int on machines with 32-bit ints and 64-bit longs.
 * #define Sudden_Underflow for IEEE-format machines without gradual
 *        underflow (i.e., that flush to zero on underflow).
 * #define IBM for IBM mainframe-style floating-point arithmetic.
 * #define VAX for VAX-style floating-point arithmetic.
 * #define Unsigned_Shifts if >> does treats its left operand as unsigned.
 * #define No_leftright to omit left-right logic in fast floating-point
 *        computation of dtoa.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3.
 * #define RND_PRODQUOT to use rnd_prod and rnd_quot (assembly routines
 *        that use extended-precision instructions to compute rounded
 *        products and quotients) with IBM.
 * #define ROUND_BIASED for IEEE-format with biased rounding.
 * #define Inaccurate_Divide for IEEE-format with correctly rounded
 *        products but inaccurate quotients, e.g., for Intel i860.
 * #define Just_16 to store 16 bits per 32-bit Long when doing high-precision
 *        integer arithmetic.  Whether this speeds things up or slows things
 *        down depends on the machine and the number being converted.
 * #define KR_headers for old-style C function headers.
 * #define Bad_float_h if your system lacks a float.h or if it does not
 *        define some or all of DBL_DIG, DBL_MAX_10_EXP, DBL_MAX_EXP,
 *        FLT_RADIX, FLT_ROUNDS, and DBL_MAX.
 * #define MALLOC your_malloc, where your_malloc(n) acts like malloc(n)
 *        if memory is available and otherwise does something you deem
 *        appropriate.  If MALLOC is undefined, malloc will be invoked
 *        directly -- and assumed always to succeed.
 */

#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: strtod.c,v 1.26 1998/02/03 18:44:21 perry Exp $");
#endif /* LIBC_SCCS and not lint */

/*
#if defined(__m68k__)    || defined(__sparc__) || defined(__i386__) || \
     defined(__mips__)    || defined(__ns32k__) || defined(__alpha__) || \
     defined(__powerpc__) || defined(Q_OS_WIN) || defined(Q_OS_DARWIN) || defined(Q_OS_MAC) || \
     defined(mips) || defined(Q_OS_AIX) || defined(Q_OS_SOLARIS)
#           define IEEE_BIG_OR_LITTLE_ENDIAN 1
#endif
*/

// *All* of our architectures have IEEE arithmetic, don't they?
#define IEEE_BIG_OR_LITTLE_ENDIAN 1

#ifdef __arm32__
/*
 * Although the CPU is little endian the FP has different
 * byte and word endianness. The byte order is still little endian
 * but the word order is big endian.
 */
#define IEEE_BIG_OR_LITTLE_ENDIAN
#endif

#ifdef vax
#define VAX
#endif

#define Long        qint32
#define ULong        quint32

#define MALLOC malloc

#ifdef BSD_QDTOA_DEBUG
#include <stdio.h>
#define Bug(x) {fprintf(stderr, "%s\n", x); exit(1);}
#endif

#ifdef Unsigned_Shifts
#define Sign_Extend(a,b) if (b < 0) a |= 0xffff0000;
#else
#define Sign_Extend(a,b) /*no-op*/
#endif

#if (defined(IEEE_BIG_OR_LITTLE_ENDIAN) + defined(VAX) + defined(IBM)) != 1
#error Exactly one of IEEE_BIG_OR_LITTLE_ENDIAN, VAX, or IBM should be defined.
#endif

static inline ULong getWord0(const NEEDS_VOLATILE double x)
{
    const NEEDS_VOLATILE uchar *ptr = reinterpret_cast<const NEEDS_VOLATILE uchar *>(&x);
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return (ptr[0]<<24) + (ptr[1]<<16) + (ptr[2]<<8) + ptr[3];
    } else {
        return (ptr[7]<<24) + (ptr[6]<<16) + (ptr[5]<<8) + ptr[4];
    }
}

static inline void setWord0(NEEDS_VOLATILE double *x, ULong l)
{
    NEEDS_VOLATILE uchar *ptr = reinterpret_cast<NEEDS_VOLATILE uchar *>(x);
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        ptr[0] = uchar(l>>24);
        ptr[1] = uchar(l>>16);
        ptr[2] = uchar(l>>8);
        ptr[3] = uchar(l);
    } else {
        ptr[7] = uchar(l>>24);
        ptr[6] = uchar(l>>16);
        ptr[5] = uchar(l>>8);
        ptr[4] = uchar(l);
    }
}

static inline ULong getWord1(const NEEDS_VOLATILE double x)
{
    const NEEDS_VOLATILE uchar *ptr = reinterpret_cast<const NEEDS_VOLATILE uchar *>(&x);
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return (ptr[4]<<24) + (ptr[5]<<16) + (ptr[6]<<8) + ptr[7];
    } else {
        return (ptr[3]<<24) + (ptr[2]<<16) + (ptr[1]<<8) + ptr[0];
    }
}
static inline void setWord1(NEEDS_VOLATILE double *x, ULong l)
{
    NEEDS_VOLATILE uchar *ptr = reinterpret_cast<uchar NEEDS_VOLATILE *>(x);
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        ptr[4] = uchar(l>>24);
        ptr[5] = uchar(l>>16);
        ptr[6] = uchar(l>>8);
        ptr[7] = uchar(l);
    } else {
        ptr[3] = uchar(l>>24);
        ptr[2] = uchar(l>>16);
        ptr[1] = uchar(l>>8);
        ptr[0] = uchar(l);
    }
}

static inline void Storeinc(ULong *&a, const ULong &b, const ULong &c)
{

    *a = (ushort(b) << 16) | ushort(c);
    ++a;
}

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#if defined(IEEE_BIG_OR_LITTLE_ENDIAN)
#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Bias 1023
#define IEEE_Arith
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define Infinite(x) (getWord0(x) == 0x7ff00000) /* sufficient test for here */
#else
#undef  Sudden_Underflow
#define Sudden_Underflow
#ifdef IBM
#define Exp_shift  24
#define Exp_shift1 24
#define Exp_msk1   0x1000000
#define Exp_msk11  0x1000000
#define Exp_mask  0x7f000000
#define P 14
#define Bias 65
#define Exp_1  0x41000000
#define Exp_11 0x41000000
#define Ebits 8        /* exponent has 7 bits, but 8 is the right value in b2d */
#define Frac_mask  0xffffff
#define Frac_mask1 0xffffff
#define Bletch 4
#define Ten_pmax 22
#define Bndry_mask  0xefffff
#define Bndry_mask1 0xffffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 4
#define Tiny0 0x100000
#define Tiny1 0
#define Quick_max 14
#define Int_max 15
#else /* VAX */
#define Exp_shift  23
#define Exp_shift1 7
#define Exp_msk1    0x80
#define Exp_msk11   0x800000
#define Exp_mask  0x7f80
#define P 56
#define Bias 129
#define Exp_1  0x40800000
#define Exp_11 0x4080
#define Ebits 8
#define Frac_mask  0x7fffff
#define Frac_mask1 0xffff007f
#define Ten_pmax 24
#define Bletch 2
#define Bndry_mask  0xffff007f
#define Bndry_mask1 0xffff007f
#define LSB 0x10000
#define Sign_bit 0x8000
#define Log2P 1
#define Tiny0 0x80
#define Tiny1 0
#define Quick_max 15
#define Int_max 15
#endif
#endif

#ifndef IEEE_Arith
#define ROUND_BIASED
#endif

#ifdef RND_PRODQUOT
#define rounded_product(a,b) a = rnd_prod(a, b)
#define rounded_quotient(a,b) a = rnd_quot(a, b)
extern double rnd_prod(double, double), rnd_quot(double, double);
#else
#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b
#endif

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

#ifndef Just_16
/* When Pack_32 is not defined, we store 16 bits per 32-bit Long.
 * This makes some inner loops simpler and sometimes saves work
 * during multiplications, but it often seems to make things slightly
 * slower.  Hence the default is now to store 32 bits per Long.
 */
#ifndef Pack_32
#define Pack_32
#endif
#endif

#define Kmax 15

struct
Bigint {
    struct Bigint *next;
    int k, maxwds, sign, wds;
    ULong x[1];
};

 typedef struct Bigint Bigint;

static Bigint *Balloc(int k)
{
    int x;
    Bigint *rv;

    x = 1 << k;
    rv = static_cast<Bigint *>(MALLOC(sizeof(Bigint) + (x-1)*sizeof(Long)));
    rv->k = k;
    rv->maxwds = x;
    rv->sign = rv->wds = 0;
    return rv;
}

static void Bfree(Bigint *v)
{
    free(v);
}

#define Bcopy(x,y) memcpy(reinterpret_cast<char *>(&x->sign), reinterpret_cast<char *>(&y->sign), \
y->wds*sizeof(Long) + 2*sizeof(int))

/* multiply by m and add a */
static Bigint *multadd(Bigint *b, int m, int a)
{
    int i, wds;
    ULong *x, y;
#ifdef Pack_32
    ULong xi, z;
#endif
    Bigint *b1;

    wds = b->wds;
    x = b->x;
    i = 0;
    do {
#ifdef Pack_32
        xi = *x;
        y = (xi & 0xffff) * m + a;
        z = (xi >> 16) * m + (y >> 16);
        a = (z >> 16);
        *x++ = (z << 16) + (y & 0xffff);
#else
        y = *x * m + a;
        a = (y >> 16);
        *x++ = y & 0xffff;
#endif
    }
    while(++i < wds);
    if (a) {
        if (wds >= b->maxwds) {
            b1 = Balloc(b->k+1);
            Bcopy(b1, b);
            Bfree(b);
            b = b1;
        }
        b->x[wds++] = a;
        b->wds = wds;
    }
    return b;
}

static Bigint *s2b(const char *s, int nd0, int nd, ULong y9)
{
    Bigint *b;
    int i, k;
    Long x, y;

    x = (nd + 8) / 9;
    for(k = 0, y = 1; x > y; y <<= 1, k++) ;
#ifdef Pack_32
    b = Balloc(k);
    b->x[0] = y9;
    b->wds = 1;
#else
    b = Balloc(k+1);
    b->x[0] = y9 & 0xffff;
    b->wds = (b->x[1] = y9 >> 16) ? 2 : 1;
#endif

    i = 9;
    if (9 < nd0) {
        s += 9;
        do b = multadd(b, 10, *s++ - '0');
        while(++i < nd0);
        s++;
    }
    else
        s += 10;
    for(; i < nd; i++)
        b = multadd(b, 10, *s++ - '0');
    return b;
}

static int hi0bits(ULong x)
{
    int k = 0;

    if (!(x & 0xffff0000)) {
        k = 16;
        x <<= 16;
    }
    if (!(x & 0xff000000)) {
        k += 8;
        x <<= 8;
    }
    if (!(x & 0xf0000000)) {
        k += 4;
        x <<= 4;
    }
    if (!(x & 0xc0000000)) {
        k += 2;
        x <<= 2;
    }
    if (!(x & 0x80000000)) {
        k++;
        if (!(x & 0x40000000))
            return 32;
    }
    return k;
}

static int lo0bits(ULong *y)
{
    int k;
    ULong x = *y;

    if (x & 7) {
        if (x & 1)
            return 0;
        if (x & 2) {
            *y = x >> 1;
            return 1;
        }
        *y = x >> 2;
        return 2;
    }
    k = 0;
    if (!(x & 0xffff)) {
        k = 16;
        x >>= 16;
    }
    if (!(x & 0xff)) {
        k += 8;
        x >>= 8;
    }
    if (!(x & 0xf)) {
        k += 4;
        x >>= 4;
    }
    if (!(x & 0x3)) {
        k += 2;
        x >>= 2;
    }
    if (!(x & 1)) {
        k++;
        x >>= 1;
        if (!x & 1)
            return 32;
    }
    *y = x;
    return k;
}

static Bigint *i2b(int i)
{
    Bigint *b;

    b = Balloc(1);
    b->x[0] = i;
    b->wds = 1;
    return b;
}

static Bigint *mult(Bigint *a, Bigint *b)
{
    Bigint *c;
    int k, wa, wb, wc;
    ULong carry, y, z;
    ULong *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
#ifdef Pack_32
    ULong z2;
#endif

    if (a->wds < b->wds) {
        c = a;
        a = b;
        b = c;
    }
    k = a->k;
    wa = a->wds;
    wb = b->wds;
    wc = wa + wb;
    if (wc > a->maxwds)
        k++;
    c = Balloc(k);
    for(x = c->x, xa = x + wc; x < xa; x++)
        *x = 0;
    xa = a->x;
    xae = xa + wa;
    xb = b->x;
    xbe = xb + wb;
    xc0 = c->x;
#ifdef Pack_32
    for(; xb < xbe; xb++, xc0++) {
        if ((y = *xb & 0xffff) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
                carry = z >> 16;
                z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
                carry = z2 >> 16;
                Storeinc(xc, z2, z);
            }
            while(x < xae);
            *xc = carry;
        }
        if ((y = *xb >> 16) != 0) {
            x = xa;
            xc = xc0;
            carry = 0;
            z2 = *xc;
            do {
                z = (*x & 0xffff) * y + (*xc >> 16) + carry;
                carry = z >> 16;
                Storeinc(xc, z, z2);
                z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
                carry = z2 >> 16;
            }
            while(x < xae);
            *xc = z2;
        }
    }
#else
    for(; xb < xbe; xc0++) {
        if (y = *xb++) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = *x++ * y + *xc + carry;
                carry = z >> 16;
                *xc++ = z & 0xffff;
            }
            while(x < xae);
            *xc = carry;
        }
    }
#endif
    for(xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) ;
    c->wds = wc;
    return c;
}

static Bigint *p5s;

static Bigint *pow5mult(Bigint *b, int k)
{
    Bigint *b1, *p5, *p51;
    int i;
    static const int p05[3] = { 5, 25, 125 };

    if ((i = k & 3) != 0)
#if defined(Q_OS_IRIX) && defined(Q_CC_GNU)
    {
        // work around a bug on 64 bit IRIX gcc
        int *p = (int *) p05;
        b = multadd(b, p[i-1], 0);
    }
#else
    b = multadd(b, p05[i-1], 0);
#endif

    if (!(k >>= 2))
        return b;
    if (!(p5 = p5s)) {
        /* first time */
        p5 = p5s = i2b(625);
        p5->next = 0;
    }
    for(;;) {
        if (k & 1) {
            b1 = mult(b, p5);
            Bfree(b);
            b = b1;
        }
        if (!(k >>= 1))
            break;
        if (!(p51 = p5->next)) {
            p51 = p5->next = mult(p5,p5);
            p51->next = 0;
        }
        p5 = p51;
    }
    return b;
}

static Bigint *lshift(Bigint *b, int k)
{
    int i, k1, n, n1;
    Bigint *b1;
    ULong *x, *x1, *xe, z;

#ifdef Pack_32
    n = k >> 5;
#else
    n = k >> 4;
#endif
    k1 = b->k;
    n1 = n + b->wds + 1;
    for(i = b->maxwds; n1 > i; i <<= 1)
        k1++;
    b1 = Balloc(k1);
    x1 = b1->x;
    for(i = 0; i < n; i++)
        *x1++ = 0;
    x = b->x;
    xe = x + b->wds;
#ifdef Pack_32
    if (k &= 0x1f) {
        k1 = 32 - k;
        z = 0;
        do {
            *x1++ = *x << k | z;
            z = *x++ >> k1;
        }
        while(x < xe);
        if ((*x1 = z) != 0)
            ++n1;
    }
#else
    if (k &= 0xf) {
        k1 = 16 - k;
        z = 0;
        do {
            *x1++ = *x << k  & 0xffff | z;
            z = *x++ >> k1;
        }
        while(x < xe);
        if (*x1 = z)
            ++n1;
    }
#endif
    else do
        *x1++ = *x++;
    while(x < xe);
    b1->wds = n1 - 1;
    Bfree(b);
    return b1;
}

static int cmp(Bigint *a, Bigint *b)
{
    ULong *xa, *xa0, *xb, *xb0;
    int i, j;

    i = a->wds;
    j = b->wds;
#ifdef BSD_QDTOA_DEBUG
    if (i > 1 && !a->x[i-1])
        Bug("cmp called with a->x[a->wds-1] == 0");
    if (j > 1 && !b->x[j-1])
        Bug("cmp called with b->x[b->wds-1] == 0");
#endif
    if (i -= j)
        return i;
    xa0 = a->x;
    xa = xa0 + j;
    xb0 = b->x;
    xb = xb0 + j;
    for(;;) {
        if (*--xa != *--xb)
            return *xa < *xb ? -1 : 1;
        if (xa <= xa0)
            break;
    }
    return 0;
}

static Bigint *diff(Bigint *a, Bigint *b)
{
    Bigint *c;
    int i, wa, wb;
    Long borrow, y;        /* We need signed shifts here. */
    ULong *xa, *xae, *xb, *xbe, *xc;
#ifdef Pack_32
    Long z;
#endif

    i = cmp(a,b);
    if (!i) {
        c = Balloc(0);
        c->wds = 1;
        c->x[0] = 0;
        return c;
    }
    if (i < 0) {
        c = a;
        a = b;
        b = c;
        i = 1;
    }
    else
        i = 0;
    c = Balloc(a->k);
    c->sign = i;
    wa = a->wds;
    xa = a->x;
    xae = xa + wa;
    wb = b->wds;
    xb = b->x;
    xbe = xb + wb;
    xc = c->x;
    borrow = 0;
#ifdef Pack_32
    do {
        y = (*xa & 0xffff) - (*xb & 0xffff) + borrow;
        borrow = y >> 16;
        Sign_Extend(borrow, y);
        z = (*xa++ >> 16) - (*xb++ >> 16) + borrow;
        borrow = z >> 16;
        Sign_Extend(borrow, z);
        Storeinc(xc, z, y);
    }
    while(xb < xbe);
    while(xa < xae) {
        y = (*xa & 0xffff) + borrow;
        borrow = y >> 16;
        Sign_Extend(borrow, y);
        z = (*xa++ >> 16) + borrow;
        borrow = z >> 16;
        Sign_Extend(borrow, z);
        Storeinc(xc, z, y);
    }
#else
    do {
        y = *xa++ - *xb++ + borrow;
        borrow = y >> 16;
        Sign_Extend(borrow, y);
        *xc++ = y & 0xffff;
    }
    while(xb < xbe);
    while(xa < xae) {
        y = *xa++ + borrow;
        borrow = y >> 16;
        Sign_Extend(borrow, y);
        *xc++ = y & 0xffff;
    }
#endif
    while(!*--xc)
        wa--;
    c->wds = wa;
    return c;
}

static double ulp(double x)
{
    Long L;
    double a;

    L = (getWord0(x) & Exp_mask) - (P-1)*Exp_msk1;
#ifndef Sudden_Underflow
    if (L > 0) {
#endif
#ifdef IBM
        L |= Exp_msk1 >> 4;
#endif
        setWord0(&a, L);
        setWord1(&a, 0);
#ifndef Sudden_Underflow
    }
    else {
        L = -L >> Exp_shift;
        if (L < Exp_shift) {
            setWord0(&a, 0x80000 >> L);
            setWord1(&a, 0);
        }
        else {
            setWord0(&a, 0);
            L -= Exp_shift;
            setWord1(&a, L >= 31 ? 1U : 1U << (31 - L));
        }
    }
#endif
    return a;
}

static double b2d(Bigint *a, int *e)
{
    ULong *xa, *xa0, w, y, z;
    int k;
    double d;

    xa0 = a->x;
    xa = xa0 + a->wds;
    y = *--xa;
#ifdef BSD_QDTOA_DEBUG
    if (!y) Bug("zero y in b2d");
#endif
    k = hi0bits(y);
    *e = 32 - k;
#ifdef Pack_32
    if (k < Ebits) {
        setWord0(&d, Exp_1 | y >> (Ebits - k));
        w = xa > xa0 ? *--xa : 0;
        setWord1(&d, y << ((32-Ebits) + k) | w >> (Ebits - k));
        goto ret_d;
    }
    z = xa > xa0 ? *--xa : 0;
    if (k -= Ebits) {
        setWord0(&d, Exp_1 | y << k | z >> (32 - k));
        y = xa > xa0 ? *--xa : 0;
        setWord1(&d, z << k | y >> (32 - k));
    }
    else {
        setWord0(&d, Exp_1 | y);
        setWord1(&d, z);
    }
#else
    if (k < Ebits + 16) {
        z = xa > xa0 ? *--xa : 0;
        setWord0(&d, Exp_1 | y << k - Ebits | z >> Ebits + 16 - k);
        w = xa > xa0 ? *--xa : 0;
        y = xa > xa0 ? *--xa : 0;
        setWord1(&d, z << k + 16 - Ebits | w << k - Ebits | y >> 16 + Ebits - k);
        goto ret_d;
    }
    z = xa > xa0 ? *--xa : 0;
    w = xa > xa0 ? *--xa : 0;
    k -= Ebits + 16;
    setWord0(&d, Exp_1 | y << k + 16 | z << k | w >> 16 - k);
    y = xa > xa0 ? *--xa : 0;
    setWord1(&d, w << k + 16 | y << k);
#endif
 ret_d:
    return d;
}

static Bigint *d2b(double d, int *e, int *bits)
{
    Bigint *b;
    int de, i, k;
    ULong *x, y, z;

#ifdef Pack_32
    b = Balloc(1);
#else
    b = Balloc(2);
#endif
    x = b->x;

    z = getWord0(d) & Frac_mask;
    setWord0(&d, getWord0(d) & 0x7fffffff);        /* clear sign bit, which we ignore */
#ifdef Sudden_Underflow
    de = (int)(getWord0(d) >> Exp_shift);
#ifndef IBM
    z |= Exp_msk11;
#endif
#else
    if ((de = int(getWord0(d) >> Exp_shift)) != 0)
        z |= Exp_msk1;
#endif
#ifdef Pack_32
    if ((y = getWord1(d)) != 0) {
        if ((k = lo0bits(&y)) != 0) {
            x[0] = y | z << (32 - k);
            z >>= k;
        }
        else
            x[0] = y;
        i = b->wds = (x[1] = z) ? 2 : 1;
    }
    else {
#ifdef BSD_QDTOA_DEBUG
        if (!z)
            Bug("Zero passed to d2b");
#endif
        k = lo0bits(&z);
        x[0] = z;
        i = b->wds = 1;
        k += 32;
    }
#else
    if (y = getWord1(d)) {
        if (k = lo0bits(&y))
            if (k >= 16) {
                x[0] = y | z << 32 - k & 0xffff;
                x[1] = z >> k - 16 & 0xffff;
                x[2] = z >> k;
                i = 2;
            }
            else {
                x[0] = y & 0xffff;
                x[1] = y >> 16 | z << 16 - k & 0xffff;
                x[2] = z >> k & 0xffff;
                x[3] = z >> k+16;
                i = 3;
            }
        else {
            x[0] = y & 0xffff;
            x[1] = y >> 16;
            x[2] = z & 0xffff;
            x[3] = z >> 16;
            i = 3;
        }
    }
    else {
#ifdef BSD_QDTOA_DEBUG
        if (!z)
            Bug("Zero passed to d2b");
#endif
        k = lo0bits(&z);
        if (k >= 16) {
            x[0] = z;
            i = 0;
        }
        else {
            x[0] = z & 0xffff;
            x[1] = z >> 16;
            i = 1;
        }
        k += 32;
    }
    while(!x[i])
        --i;
    b->wds = i + 1;
#endif
#ifndef Sudden_Underflow
    if (de) {
#endif
#ifdef IBM
        *e = (de - Bias - (P-1) << 2) + k;
        *bits = 4*P + 8 - k - hi0bits(getWord0(d) & Frac_mask);
#else
        *e = de - Bias - (P-1) + k;
        *bits = P - k;
#endif
#ifndef Sudden_Underflow
    }
    else {
        *e = de - Bias - (P-1) + 1 + k;
#ifdef Pack_32
        *bits = 32*i - hi0bits(x[i-1]);
#else
        *bits = (i+2)*16 - hi0bits(x[i]);
#endif
    }
#endif
    return b;
}

static double ratio(Bigint *a, Bigint *b)
{
    double da, db;
    int k, ka, kb;

    da = b2d(a, &ka);
    db = b2d(b, &kb);
#ifdef Pack_32
    k = ka - kb + 32*(a->wds - b->wds);
#else
    k = ka - kb + 16*(a->wds - b->wds);
#endif
#ifdef IBM
    if (k > 0) {
        setWord0(&da, getWord0(da) + (k >> 2)*Exp_msk1);
        if (k &= 3)
            da *= 1 << k;
    }
    else {
        k = -k;
        setWord0(&db, getWord0(db) + (k >> 2)*Exp_msk1);
        if (k &= 3)
            db *= 1 << k;
    }
#else
    if (k > 0)
        setWord0(&da, getWord0(da) + k*Exp_msk1);
    else {
        k = -k;
        setWord0(&db, getWord0(db) + k*Exp_msk1);
    }
#endif
    return da / db;
}

static const double tens[] = {
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
    1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
    1e20, 1e21, 1e22
#ifdef VAX
    , 1e23, 1e24
#endif
};

#ifdef IEEE_Arith
static const double bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static const double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128, 1e-256 };
#define n_bigtens 5
#else
#ifdef IBM
static const double bigtens[] = { 1e16, 1e32, 1e64 };
static const double tinytens[] = { 1e-16, 1e-32, 1e-64 };
#define n_bigtens 3
#else
static const double bigtens[] = { 1e16, 1e32 };
static const double tinytens[] = { 1e-16, 1e-32 };
#define n_bigtens 2
#endif
#endif

/*
  The pre-release gcc3.3 shipped with SuSE 8.2 has a bug which causes
  the comparison 1e-100 == 0.0 to return true. As a workaround, we
  compare it to a global variable containing 0.0, which produces
  correct assembler output.

  ### consider detecting the broken compilers and using the static
  ### double for these, and use a #define for all working compilers
*/
static double g_double_zero = 0.0;

static double qstrtod(const char *s00, const char **se, bool *ok)
{
    int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, dsign,
        e, e1, esign, i, j, k, nd, nd0, nf, nz, nz0, sign;
    const char *s, *s0, *s1;
    double aadj, aadj1, adj, rv, rv0;
    Long L;
    ULong y, z;
    Bigint *bb1, *bd0;
    Bigint *bb = NULL, *bd = NULL, *bs = NULL, *delta = NULL;/* pacify gcc */

    /*
      #ifndef KR_headers
      const char decimal_point = localeconv()->decimal_point[0];
      #else
      const char decimal_point = '.';
      #endif */
    if (ok != 0)
        *ok = true;

    const char decimal_point = '.';

    sign = nz0 = nz = 0;
    rv = 0.;


    for(s = s00; isspace(uchar(*s)); s++)
        ;

    if (*s == '-') {
        sign = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (*s == '\0') {
        s = s00;
        goto ret;
    }

    if (*s == '0') {
        nz0 = 1;
        while(*++s == '0') ;
        if (!*s)
            goto ret;
    }
    s0 = s;
    y = z = 0;
    for(nd = nf = 0; (c = *s) >= '0' && c <= '9'; nd++, s++)
        if (nd < 9)
            y = 10*y + c - '0';
        else if (nd < 16)
            z = 10*z + c - '0';
    nd0 = nd;
    if (c == decimal_point) {
        c = *++s;
        if (!nd) {
            for(; c == '0'; c = *++s)
                nz++;
            if (c > '0' && c <= '9') {
                s0 = s;
                nf += nz;
                nz = 0;
                goto have_dig;
            }
            goto dig_done;
        }
        for(; c >= '0' && c <= '9'; c = *++s) {
        have_dig:
            nz++;
            if (c -= '0') {
                nf += nz;
                for(i = 1; i < nz; i++)
                    if (nd++ < 9)
                        y *= 10;
                    else if (nd <= DBL_DIG + 1)
                        z *= 10;
                if (nd++ < 9)
                    y = 10*y + c;
                else if (nd <= DBL_DIG + 1)
                    z = 10*z + c;
                nz = 0;
            }
        }
    }
 dig_done:
    e = 0;
    if (c == 'e' || c == 'E') {
        if (!nd && !nz && !nz0) {
            s = s00;
            goto ret;
        }
        s00 = s;
        esign = 0;
        switch(c = *++s) {
        case '-':
            esign = 1;
        case '+':
            c = *++s;
        }
        if (c >= '0' && c <= '9') {
            while(c == '0')
                c = *++s;
            if (c > '0' && c <= '9') {
                L = c - '0';
                s1 = s;
                while((c = *++s) >= '0' && c <= '9')
                    L = 10*L + c - '0';
                if (s - s1 > 8 || L > 19999)
                    /* Avoid confusion from exponents
                     * so large that e might overflow.
                     */
                    e = 19999; /* safe for 16 bit ints */
                else
                    e = int(L);
                if (esign)
                    e = -e;
            }
            else
                e = 0;
        }
        else
            s = s00;
    }
    if (!nd) {
        if (!nz && !nz0)
            s = s00;
        goto ret;
    }
    e1 = e -= nf;

    /* Now we have nd0 digits, starting at s0, followed by a
     * decimal point, followed by nd-nd0 digits.  The number we're
     * after is the integer represented by those digits times
     * 10**e */

    if (!nd0)
        nd0 = nd;
    k = nd < DBL_DIG + 1 ? nd : DBL_DIG + 1;
    rv = y;
    if (k > 9)
#if defined(Q_OS_IRIX) && defined(Q_CC_GNU)
    {
        // work around a bug on 64 bit IRIX gcc
        double *t = (double *) tens;
        rv = t[k - 9] * rv + z;
    }
#else
    rv = tens[k - 9] * rv + z;
#endif

    bd0 = 0;
    if (nd <= DBL_DIG
#ifndef RND_PRODQUOT
        && FLT_ROUNDS == 1
#endif
        ) {
        if (!e)
            goto ret;
        if (e > 0) {
            if (e <= Ten_pmax) {
#ifdef VAX
                goto vax_ovfl_check;
#else
                /* rv = */ rounded_product(rv, tens[e]);
                goto ret;
#endif
            }
            i = DBL_DIG - nd;
            if (e <= Ten_pmax + i) {
                /* A fancier test would sometimes let us do
                 * this for larger i values.
                 */
                e -= i;
                rv *= tens[i];
#ifdef VAX
                /* VAX exponent range is so narrow we must
                 * worry about overflow here...
                 */
            vax_ovfl_check:
                setWord0(&rv, getWord0(rv) - P*Exp_msk1);
                /* rv = */ rounded_product(rv, tens[e]);
                if ((getWord0(rv) & Exp_mask)
                    > Exp_msk1*(DBL_MAX_EXP+Bias-1-P))
                    goto ovfl;
                setWord0(&rv, getWord0(rv) + P*Exp_msk1);
#else
                /* rv = */ rounded_product(rv, tens[e]);
#endif
                goto ret;
            }
        }
#ifndef Inaccurate_Divide
        else if (e >= -Ten_pmax) {
            /* rv = */ rounded_quotient(rv, tens[-e]);
            goto ret;
        }
#endif
    }
    e1 += nd - k;

    /* Get starting approximation = rv * 10**e1 */

    if (e1 > 0) {
        if ((i = e1 & 15) != 0)
            rv *= tens[i];
        if (e1 &= ~15) {
            if (e1 > DBL_MAX_10_EXP) {
            ovfl:
                //                                errno = ERANGE;
                if (ok != 0)
                    *ok = false;
#ifdef __STDC__
                rv = HUGE_VAL;
#else
                /* Can't trust HUGE_VAL */
#ifdef IEEE_Arith
                setWord0(&rv, Exp_mask);
                setWord1(&rv, 0);
#else
                setWord0(&rv, Big0);
                setWord1(&rv, Big1);
#endif
#endif
                if (bd0)
                    goto retfree;
                goto ret;
            }
            if (e1 >>= 4) {
                for(j = 0; e1 > 1; j++, e1 >>= 1)
                    if (e1 & 1)
                        rv *= bigtens[j];
                /* The last multiplication could overflow. */
                setWord0(&rv, getWord0(rv) - P*Exp_msk1);
                rv *= bigtens[j];
                if ((z = getWord0(rv) & Exp_mask)
                    > Exp_msk1*(DBL_MAX_EXP+Bias-P))
                    goto ovfl;
                if (z > Exp_msk1*(DBL_MAX_EXP+Bias-1-P)) {
                    /* set to largest number */
                    /* (Can't trust DBL_MAX) */
                    setWord0(&rv, Big0);
                    setWord1(&rv, Big1);
                }
                else
                    setWord0(&rv, getWord0(rv) + P*Exp_msk1);
            }

        }
    }
    else if (e1 < 0) {
        e1 = -e1;
        if ((i = e1 & 15) != 0)
            rv /= tens[i];
        if (e1 &= ~15) {
            e1 >>= 4;
            if (e1 >= 1 << n_bigtens)
                goto undfl;
            for(j = 0; e1 > 1; j++, e1 >>= 1)
                if (e1 & 1)
                    rv *= tinytens[j];
            /* The last multiplication could underflow. */
            rv0 = rv;
            rv *= tinytens[j];
            if (rv == g_double_zero)
                {
                    rv = 2.*rv0;
                    rv *= tinytens[j];
                    if (rv == g_double_zero)
                        {
                        undfl:
                            rv = 0.;
                            //                                        errno = ERANGE;
                            if (ok != 0)
                                *ok = false;
                            if (bd0)
                                goto retfree;
                            goto ret;
                        }
                    setWord0(&rv, Tiny0);
                    setWord1(&rv, Tiny1);
                    /* The refinement below will clean
                     * this approximation up.
                     */
                }
        }
    }

    /* Now the hard part -- adjusting rv to the correct value.*/

    /* Put digits into bd: true value = bd * 10^e */

    bd0 = s2b(s0, nd0, nd, y);

    for(;;) {
        bd = Balloc(bd0->k);
        Bcopy(bd, bd0);
        bb = d2b(rv, &bbe, &bbbits);        /* rv = bb * 2^bbe */
        bs = i2b(1);

        if (e >= 0) {
            bb2 = bb5 = 0;
            bd2 = bd5 = e;
        }
        else {
            bb2 = bb5 = -e;
            bd2 = bd5 = 0;
        }
        if (bbe >= 0)
            bb2 += bbe;
        else
            bd2 -= bbe;
        bs2 = bb2;
#ifdef Sudden_Underflow
#ifdef IBM
        j = 1 + 4*P - 3 - bbbits + ((bbe + bbbits - 1) & 3);
#else
        j = P + 1 - bbbits;
#endif
#else
        i = bbe + bbbits - 1;        /* logb(rv) */
        if (i < Emin)        /* denormal */
            j = bbe + (P-Emin);
        else
            j = P + 1 - bbbits;
#endif
        bb2 += j;
        bd2 += j;
        i = bb2 < bd2 ? bb2 : bd2;
        if (i > bs2)
            i = bs2;
        if (i > 0) {
            bb2 -= i;
            bd2 -= i;
            bs2 -= i;
        }
        if (bb5 > 0) {
            bs = pow5mult(bs, bb5);
            bb1 = mult(bs, bb);
            Bfree(bb);
            bb = bb1;
        }
        if (bb2 > 0)
            bb = lshift(bb, bb2);
        if (bd5 > 0)
            bd = pow5mult(bd, bd5);
        if (bd2 > 0)
            bd = lshift(bd, bd2);
        if (bs2 > 0)
            bs = lshift(bs, bs2);
        delta = diff(bb, bd);
        dsign = delta->sign;
        delta->sign = 0;
        i = cmp(delta, bs);
        if (i < 0) {
            /* Error is less than half an ulp -- check for
             * special case of mantissa a power of two.
             */
            if (dsign || getWord1(rv) || getWord0(rv) & Bndry_mask)
                break;
            delta = lshift(delta,Log2P);
            if (cmp(delta, bs) > 0)
                goto drop_down;
            break;
        }
        if (i == 0) {
            /* exactly half-way between */
            if (dsign) {
                if ((getWord0(rv) & Bndry_mask1) == Bndry_mask1
                    &&  getWord1(rv) == 0xffffffff) {
                    /*boundary case -- increment exponent*/
                    setWord0(&rv, (getWord0(rv) & Exp_mask)
                                + Exp_msk1
#ifdef IBM
                                | Exp_msk1 >> 4
#endif
                                );
                    setWord1(&rv, 0);
                    break;
                }
            }
            else if (!(getWord0(rv) & Bndry_mask) && !getWord1(rv)) {
            drop_down:
                /* boundary case -- decrement exponent */
#ifdef Sudden_Underflow
                L = getWord0(rv) & Exp_mask;
#ifdef IBM
                if (L <  Exp_msk1)
#else
                    if (L <= Exp_msk1)
#endif
                        goto undfl;
                L -= Exp_msk1;
#else
                L = (getWord0(rv) & Exp_mask) - Exp_msk1;
#endif
                setWord0(&rv, L | Bndry_mask1);
                setWord1(&rv, 0xffffffff);
#ifdef IBM
                goto cont;
#else
                break;
#endif
            }
#ifndef ROUND_BIASED
            if (!(getWord1(rv) & LSB))
                break;
#endif
            if (dsign)
                rv += ulp(rv);
#ifndef ROUND_BIASED
            else {
                rv -= ulp(rv);
#ifndef Sudden_Underflow
                if (rv == g_double_zero)
                    goto undfl;
#endif
            }
#endif
            break;
        }
        if ((aadj = ratio(delta, bs)) <= 2.) {
            if (dsign)
                aadj = aadj1 = 1.;
            else if (getWord1(rv) || getWord0(rv) & Bndry_mask) {
#ifndef Sudden_Underflow
                if (getWord1(rv) == Tiny1 && !getWord0(rv))
                    goto undfl;
#endif
                aadj = 1.;
                aadj1 = -1.;
            }
            else {
                /* special case -- power of FLT_RADIX to be */
                /* rounded down... */

                if (aadj < 2./FLT_RADIX)
                    aadj = 1./FLT_RADIX;
                else
                    aadj *= 0.5;
                aadj1 = -aadj;
            }
        }
        else {
            aadj *= 0.5;
            aadj1 = dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
            switch(FLT_ROUNDS) {
            case 2: /* towards +infinity */
                aadj1 -= 0.5;
                break;
            case 0: /* towards 0 */
            case 3: /* towards -infinity */
                aadj1 += 0.5;
            }
#else
            if (FLT_ROUNDS == 0)
                aadj1 += 0.5;
#endif
        }
        y = getWord0(rv) & Exp_mask;

        /* Check for overflow */

        if (y == Exp_msk1*(DBL_MAX_EXP+Bias-1)) {
            rv0 = rv;
            setWord0(&rv, getWord0(rv) - P*Exp_msk1);
            adj = aadj1 * ulp(rv);
            rv += adj;
            if ((getWord0(rv) & Exp_mask) >=
                Exp_msk1*(DBL_MAX_EXP+Bias-P)) {
                if (getWord0(rv0) == Big0 && getWord1(rv0) == Big1)
                    goto ovfl;
                setWord0(&rv, Big0);
                setWord1(&rv, Big1);
                goto cont;
            }
            else
                setWord0(&rv, getWord0(rv) + P*Exp_msk1);
        }
        else {
#ifdef Sudden_Underflow
            if ((getWord0(rv) & Exp_mask) <= P*Exp_msk1) {
                rv0 = rv;
                setWord0(&rv, getWord0(rv) + P*Exp_msk1);
                adj = aadj1 * ulp(rv);
                rv += adj;
#ifdef IBM
                if ((getWord0(rv) & Exp_mask) <  P*Exp_msk1)
#else
                    if ((getWord0(rv) & Exp_mask) <= P*Exp_msk1)
#endif
                        {
                            if (getWord0(rv0) == Tiny0
                                && getWord1(rv0) == Tiny1)
                                goto undfl;
                            setWord0(&rv, Tiny0);
                            setWord1(&rv, Tiny1);
                            goto cont;
                        }
                    else
                        setWord0(&rv, getWord0(rv) - P*Exp_msk1);
            }
            else {
                adj = aadj1 * ulp(rv);
                rv += adj;
            }
#else
            /* Compute adj so that the IEEE rounding rules will
             * correctly round rv + adj in some half-way cases.
             * If rv * ulp(rv) is denormalized (i.e.,
             * y <= (P-1)*Exp_msk1), we must adjust aadj to avoid
             * trouble from bits lost to denormalization;
             * example: 1.2e-307 .
             */
            if (y <= (P-1)*Exp_msk1 && aadj >= 1.) {
                aadj1 = int(aadj + 0.5);
                if (!dsign)
                    aadj1 = -aadj1;
            }
            adj = aadj1 * ulp(rv);
            rv += adj;
#endif
        }
        z = getWord0(rv) & Exp_mask;
        if (y == z) {
            /* Can we stop now? */
            L = Long(aadj);
            aadj -= L;
            /* The tolerances below are conservative. */
            if (dsign || getWord1(rv) || getWord0(rv) & Bndry_mask) {
                if (aadj < .4999999 || aadj > .5000001)
                    break;
            }
            else if (aadj < .4999999/FLT_RADIX)
                break;
        }
    cont:
        Bfree(bb);
        Bfree(bd);
        Bfree(bs);
        Bfree(delta);
    }
 retfree:
    Bfree(bb);
    Bfree(bd);
    Bfree(bs);
    Bfree(bd0);
    Bfree(delta);
 ret:
    if (se)
        *se = s;
    return sign ? -rv : rv;
}

static int quorem(Bigint *b, Bigint *S)
{
    int n;
    Long borrow, y;
    ULong carry, q, ys;
    ULong *bx, *bxe, *sx, *sxe;
#ifdef Pack_32
    Long z;
    ULong si, zs;
#endif

    n = S->wds;
#ifdef BSD_QDTOA_DEBUG
    /*debug*/ if (b->wds > n)
        /*debug*/        Bug("oversize b in quorem");
#endif
    if (b->wds < n)
        return 0;
    sx = S->x;
    sxe = sx + --n;
    bx = b->x;
    bxe = bx + n;
    q = *bxe / (*sxe + 1);        /* ensure q <= true quotient */
#ifdef BSD_QDTOA_DEBUG
    /*debug*/ if (q > 9)
        /*debug*/        Bug("oversized quotient in quorem");
#endif
    if (q) {
        borrow = 0;
        carry = 0;
        do {
#ifdef Pack_32
            si = *sx++;
            ys = (si & 0xffff) * q + carry;
            zs = (si >> 16) * q + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            Sign_Extend(borrow, y);
            z = (*bx >> 16) - (zs & 0xffff) + borrow;
            borrow = z >> 16;
            Sign_Extend(borrow, z);
            Storeinc(bx, z, y);
#else
            ys = *sx++ * q + carry;
            carry = ys >> 16;
            y = *bx - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            Sign_Extend(borrow, y);
            *bx++ = y & 0xffff;
#endif
        }
        while(sx <= sxe);
        if (!*bxe) {
            bx = b->x;
            while(--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    if (cmp(b, S) >= 0) {
        q++;
        borrow = 0;
        carry = 0;
        bx = b->x;
        sx = S->x;
        do {
#ifdef Pack_32
            si = *sx++;
            ys = (si & 0xffff) + carry;
            zs = (si >> 16) + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            Sign_Extend(borrow, y);
            z = (*bx >> 16) - (zs & 0xffff) + borrow;
            borrow = z >> 16;
            Sign_Extend(borrow, z);
            Storeinc(bx, z, y);
#else
            ys = *sx++ + carry;
            carry = ys >> 16;
            y = *bx - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            Sign_Extend(borrow, y);
            *bx++ = y & 0xffff;
#endif
        }
        while(sx <= sxe);
        bx = b->x;
        bxe = bx + n;
        if (!*bxe) {
            while(--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    return q;
}

/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *        1. Rather than iterating, we use a simple numeric overestimate
 *           to determine k = floor(log10(d)).  We scale relevant
 *           quantities using O(log2(k)) rather than O(k) multiplications.
 *        2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *           try to generate digits strictly left to right.  Instead, we
 *           compute with fewer bits and propagate the carry if necessary
 *           when rounding the final digit up.  This is often faster.
 *        3. Under the assumption that input will be rounded nearest,
 *           mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *           That is, we allow equality in stopping tests when the
 *           round-nearest rule will give the same floating-point value
 *           as would satisfaction of the stopping test with strict
 *           inequality.
 *        4. We remove common factors of powers of 2 from relevant
 *           quantities.
 *        5. When converting floating-point integers less than 1e16,
 *           we use floating-point arithmetic rather than resorting
 *           to multiple-precision integers.
 *        6. When asked to produce fewer than 15 digits, we first try
 *           to get by with floating-point arithmetic; we resort to
 *           multiple-precision integer arithmetic only if we cannot
 *           guarantee that the floating-point calculation has given
 *           the correctly rounded result.  For k requested digits and
 *           "uniformly" distributed input, the probability is
 *           something like 10^(k-15) that we must resort to the Long
 *           calculation.
 */


/* This actually sometimes returns a pointer to a string literal
   cast to a char*. Do NOT try to modify the return value. */

static char *qdtoa ( double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **resultp)
{
    // Some values of the floating-point control word can cause _qdtoa to crash with an underflow.
    // We set a safe value here.
#ifdef Q_OS_WIN
    _clear87();
    unsigned int oldbits = _control87(0, 0);
#ifndef MCW_EM
#    ifdef _MCW_EM
#        define MCW_EM _MCW_EM
#    else
#        define MCW_EM 0x0008001F
#    endif
#endif
    _control87(MCW_EM, MCW_EM);
#endif

#if defined(Q_OS_LINUX) && !defined(__UCLIBC__)
    fenv_t envp;
    feholdexcept(&envp);
#endif

    char *s = _qdtoa(d, mode, ndigits, decpt, sign, rve, resultp);

#ifdef Q_OS_WIN
    _clear87();
#ifndef _M_X64
    _control87(oldbits, 0xFFFFF);
#else
    _control87(oldbits, _MCW_EM|_MCW_DN|_MCW_RC);
#endif //_M_X64
#endif //Q_OS_WIN

#if defined(Q_OS_LINUX) && !defined(__UCLIBC__)
    fesetenv(&envp);
#endif

    return s;
}

static char *_qdtoa( NEEDS_VOLATILE double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **resultp)
{
    /*
      Arguments ndigits, decpt, sign are similar to those
      of ecvt and fcvt; trailing zeros are suppressed from
      the returned string.  If not null, *rve is set to point
      to the end of the return value.  If d is +-Infinity or NaN,
      then *decpt is set to 9999.

      mode:
      0 ==> shortest string that yields d when read in
      and rounded to nearest.
      1 ==> like 0, but with Steele & White stopping rule;
      e.g. with IEEE P754 arithmetic , mode 0 gives
      1e23 whereas mode 1 gives 9.999999999999999e22.
      2 ==> max(1,ndigits) significant digits.  This gives a
      return value similar to that of ecvt, except
      that trailing zeros are suppressed.
      3 ==> through ndigits past the decimal point.  This
      gives a return value similar to that from fcvt,
      except that trailing zeros are suppressed, and
      ndigits can be negative.
      4-9 should give the same return values as 2-3, i.e.,
      4 <= mode <= 9 ==> same return as mode
      2 + (mode & 1).  These modes are mainly for
      debugging; often they run slower but sometimes
      faster than modes 2-3.
      4,5,8,9 ==> left-to-right digit generation.
      6-9 ==> don't try fast floating-point estimate
      (if applicable).

      Values of mode other than 0-9 are treated as mode 0.

      Sufficient space is allocated to the return value
      to hold the suppressed trailing zeros.
    */

    int bbits, b2, b5, be, dig, i, ieps, ilim0,
        j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
        try_quick;
    int ilim = 0, ilim1 = 0, spec_case = 0;        /* pacify gcc */
    Long L;
#ifndef Sudden_Underflow
    int denorm;
    ULong x;
#endif
    Bigint *b, *b1, *delta, *mhi, *S;
    Bigint *mlo = NULL; /* pacify gcc */
    double d2;
    double ds, eps;
    char *s, *s0;

    if (getWord0(d) & Sign_bit) {
        /* set sign for everything, including 0's and NaNs */
        *sign = 1;
        setWord0(&d, getWord0(d) & ~Sign_bit);        /* clear sign bit */
    }
    else
        *sign = 0;

#if defined(IEEE_Arith) + defined(VAX)
#ifdef IEEE_Arith
    if ((getWord0(d) & Exp_mask) == Exp_mask)
#else
        if (getWord0(d)  == 0x8000)
#endif
            {
                /* Infinity or NaN */
                *decpt = 9999;
                s =
#ifdef IEEE_Arith
                    !getWord1(d) && !(getWord0(d) & 0xfffff) ? const_cast<char*>("Infinity") :
#endif
                    const_cast<char*>("NaN");
                if (rve)
                    *rve =
#ifdef IEEE_Arith
                        s[3] ? s + 8 :
#endif
                        s + 3;
                return s;
            }
#endif
#ifdef IBM
    d += 0; /* normalize */
#endif
    if (d == g_double_zero)
        {
            *decpt = 1;
            s = const_cast<char*>("0");
            if (rve)
                *rve = s + 1;
            return s;
        }

    b = d2b(d, &be, &bbits);
#ifdef Sudden_Underflow
    i = (int)(getWord0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1));
#else
    if ((i = int(getWord0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1))) != 0) {
#endif
        d2 = d;
        setWord0(&d2, getWord0(d2) & Frac_mask1);
        setWord0(&d2, getWord0(d2) | Exp_11);
#ifdef IBM
        if (j = 11 - hi0bits(getWord0(d2) & Frac_mask))
            d2 /= 1 << j;
#endif

        /* log(x)        ~=~ log(1.5) + (x-1.5)/1.5
         * log10(x)         =  log(x) / log(10)
         *                ~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
         * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
         *
         * This suggests computing an approximation k to log10(d) by
         *
         * k = (i - Bias)*0.301029995663981
         *        + ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
         *
         * We want k to be too large rather than too small.
         * The error in the first-order Taylor series approximation
         * is in our favor, so we just round up the constant enough
         * to compensate for any error in the multiplication of
         * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
         * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
         * adding 1e-13 to the constant term more than suffices.
         * Hence we adjust the constant term to 0.1760912590558.
         * (We could get a more accurate k by invoking log10,
         *  but this is probably not worthwhile.)
         */

        i -= Bias;
#ifdef IBM
        i <<= 2;
        i += j;
#endif
#ifndef Sudden_Underflow
        denorm = 0;
    }
    else {
        /* d is denormalized */

        i = bbits + be + (Bias + (P-1) - 1);
        x = i > 32  ? getWord0(d) << (64 - i) | getWord1(d) >> (i - 32)
            : getWord1(d) << (32 - i);
        d2 = x;
        setWord0(&d2, getWord0(d2) - 31*Exp_msk1); /* adjust exponent */
        i -= (Bias + (P-1) - 1) + 1;
        denorm = 1;
    }
#endif
    ds = (d2-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
    k = int(ds);
    if (ds < 0. && ds != k)
        k--;        /* want k = floor(ds) */
    k_check = 1;
    if (k >= 0 && k <= Ten_pmax) {
        if (d < tens[k])
            k--;
        k_check = 0;
    }
    j = bbits - i - 1;
    if (j >= 0) {
        b2 = 0;
        s2 = j;
    }
    else {
        b2 = -j;
        s2 = 0;
    }
    if (k >= 0) {
        b5 = 0;
        s5 = k;
        s2 += k;
    }
    else {
        b2 -= k;
        b5 = -k;
        s5 = 0;
    }
    if (mode < 0 || mode > 9)
        mode = 0;
    try_quick = 1;
    if (mode > 5) {
        mode -= 4;
        try_quick = 0;
    }
    leftright = 1;
    switch(mode) {
    case 0:
    case 1:
        ilim = ilim1 = -1;
        i = 18;
        ndigits = 0;
        break;
    case 2:
        leftright = 0;
        /* no break */
    case 4:
        if (ndigits <= 0)
            ndigits = 1;
        ilim = ilim1 = i = ndigits;
        break;
    case 3:
        leftright = 0;
        /* no break */
    case 5:
        i = ndigits + k + 1;
        ilim = i;
        ilim1 = i - 1;
        if (i <= 0)
            i = 1;
    }
    *resultp = static_cast<char *>(malloc(i + 1));
    s = s0 = *resultp;

    if (ilim >= 0 && ilim <= Quick_max && try_quick) {

        /* Try to get by with floating-point arithmetic. */

        i = 0;
        d2 = d;
        k0 = k;
        ilim0 = ilim;
        ieps = 2; /* conservative */
        if (k > 0) {
            ds = tens[k&0xf];
            j = k >> 4;
            if (j & Bletch) {
                /* prevent overflows */
                j &= Bletch - 1;
                d /= bigtens[n_bigtens-1];
                ieps++;
            }
            for(; j; j >>= 1, i++)
                if (j & 1) {
                    ieps++;
                    ds *= bigtens[i];
                }
            d /= ds;
        }
        else if ((j1 = -k) != 0) {
            d *= tens[j1 & 0xf];
            for(j = j1 >> 4; j; j >>= 1, i++)
                if (j & 1) {
                    ieps++;
                    d *= bigtens[i];
                }
        }
        if (k_check && d < 1. && ilim > 0) {
            if (ilim1 <= 0)
                goto fast_failed;
            ilim = ilim1;
            k--;
            d *= 10.;
            ieps++;
        }
        eps = ieps*d + 7.;
        setWord0(&eps, getWord0(eps) - (P-1)*Exp_msk1);
        if (ilim == 0) {
            S = mhi = 0;
            d -= 5.;
            if (d > eps)
                goto one_digit;
            if (d < -eps)
                goto no_digits;
            goto fast_failed;
        }
#ifndef No_leftright
        if (leftright) {
            /* Use Steele & White method of only
             * generating digits needed.
             */
            eps = 0.5/tens[ilim-1] - eps;
            for(i = 0;;) {
                L = Long(d);
                d -= L;
                *s++ = '0' + int(L);
                if (d < eps)
                    goto ret1;
                if (1. - d < eps)
                    goto bump_up;
                if (++i >= ilim)
                    break;
                eps *= 10.;
                d *= 10.;
            }
        }
        else {
#endif
            /* Generate ilim digits, then fix them up. */
#if defined(Q_OS_IRIX) && defined(Q_CC_GNU)
            // work around a bug on 64 bit IRIX gcc
            double *t = (double *) tens;
            eps *= t[ilim-1];
#else
            eps *= tens[ilim-1];
#endif
            for(i = 1;; i++, d *= 10.) {
                L = Long(d);
                d -= L;
                *s++ = '0' + int(L);
                if (i == ilim) {
                    if (d > 0.5 + eps)
                        goto bump_up;
                    else if (d < 0.5 - eps) {
                        while(*--s == '0');
                        s++;
                        goto ret1;
                    }
                    break;
                }
            }
#ifndef No_leftright
        }
#endif
    fast_failed:
        s = s0;
        d = d2;
        k = k0;
        ilim = ilim0;
    }

    /* Do we have a "small" integer? */

    if (be >= 0 && k <= Int_max) {
        /* Yes. */
        ds = tens[k];
        if (ndigits < 0 && ilim <= 0) {
            S = mhi = 0;
            if (ilim < 0 || d <= 5*ds)
                goto no_digits;
            goto one_digit;
        }
        for(i = 1;; i++) {
            L = Long(d / ds);
            d -= L*ds;
#ifdef Check_FLT_ROUNDS
            /* If FLT_ROUNDS == 2, L will usually be high by 1 */
            if (d < 0) {
                L--;
                d += ds;
            }
#endif
            *s++ = '0' + int(L);
            if (i == ilim) {
                d += d;
                if (d > ds || (d == ds && L & 1)) {
                bump_up:
                    while(*--s == '9')
                        if (s == s0) {
                            k++;
                            *s = '0';
                            break;
                        }
                    ++*s++;
                }
                break;
            }
            if ((d *= 10.) == g_double_zero)
                break;
        }
        goto ret1;
    }

    m2 = b2;
    m5 = b5;
    mhi = mlo = 0;
    if (leftright) {
        if (mode < 2) {
            i =
#ifndef Sudden_Underflow
                denorm ? be + (Bias + (P-1) - 1 + 1) :
#endif
#ifdef IBM
                1 + 4*P - 3 - bbits + ((bbits + be - 1) & 3);
#else
            1 + P - bbits;
#endif
        }
        else {
            j = ilim - 1;
            if (m5 >= j)
                m5 -= j;
            else {
                s5 += j -= m5;
                b5 += j;
                m5 = 0;
            }
            if ((i = ilim) < 0) {
                m2 -= i;
                i = 0;
            }
        }
        b2 += i;
        s2 += i;
        mhi = i2b(1);
    }
    if (m2 > 0 && s2 > 0) {
        i = m2 < s2 ? m2 : s2;
        b2 -= i;
        m2 -= i;
        s2 -= i;
    }
    if (b5 > 0) {
        if (leftright) {
            if (m5 > 0) {
                mhi = pow5mult(mhi, m5);
                b1 = mult(mhi, b);
                Bfree(b);
                b = b1;
            }
            if ((j = b5 - m5) != 0)
                b = pow5mult(b, j);
        }
        else
            b = pow5mult(b, b5);
    }
    S = i2b(1);
    if (s5 > 0)
        S = pow5mult(S, s5);

    /* Check for special case that d is a normalized power of 2. */

    if (mode < 2) {
        if (!getWord1(d) && !(getWord0(d) & Bndry_mask)
#ifndef Sudden_Underflow
            && getWord0(d) & Exp_mask
#endif
            ) {
            /* The special case */
            b2 += Log2P;
            s2 += Log2P;
            spec_case = 1;
        }
        else
            spec_case = 0;
    }

    /* Arrange for convenient computation of quotients:
     * shift left if necessary so divisor has 4 leading 0 bits.
     *
     * Perhaps we should just compute leading 28 bits of S once
     * and for all and pass them and a shift to quorem, so it
     * can do shifts and ors to compute the numerator for q.
     */
#ifdef Pack_32
    if ((i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0x1f) != 0)
        i = 32 - i;
#else
    if (i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0xf)
        i = 16 - i;
#endif
    if (i > 4) {
        i -= 4;
        b2 += i;
        m2 += i;
        s2 += i;
    }
    else if (i < 4) {
        i += 28;
        b2 += i;
        m2 += i;
        s2 += i;
    }
    if (b2 > 0)
        b = lshift(b, b2);
    if (s2 > 0)
        S = lshift(S, s2);
    if (k_check) {
        if (cmp(b,S) < 0) {
            k--;
            b = multadd(b, 10, 0);        /* we botched the k estimate */
            if (leftright)
                mhi = multadd(mhi, 10, 0);
            ilim = ilim1;
        }
    }
    if (ilim <= 0 && mode > 2) {
        if (ilim < 0 || cmp(b,S = multadd(S,5,0)) <= 0) {
            /* no digits, fcvt style */
        no_digits:
            k = -1 - ndigits;
            goto ret;
        }
    one_digit:
        *s++ = '1';
        k++;
        goto ret;
    }
    if (leftright) {
        if (m2 > 0)
            mhi = lshift(mhi, m2);

        /* Compute mlo -- check for special case
         * that d is a normalized power of 2.
         */

        mlo = mhi;
        if (spec_case) {
            mhi = Balloc(mhi->k);
            Bcopy(mhi, mlo);
            mhi = lshift(mhi, Log2P);
        }

        for(i = 1;;i++) {
            dig = quorem(b,S) + '0';
            /* Do we yet have the shortest decimal string
             * that will round to d?
             */
            j = cmp(b, mlo);
            delta = diff(S, mhi);
            j1 = delta->sign ? 1 : cmp(b, delta);
            Bfree(delta);
#ifndef ROUND_BIASED
            if (j1 == 0 && !mode && !(getWord1(d) & 1)) {
                if (dig == '9')
                    goto round_9_up;
                if (j > 0)
                    dig++;
                *s++ = dig;
                goto ret;
            }
#endif
            if (j < 0 || (j == 0 && !mode
#ifndef ROUND_BIASED
                          && !(getWord1(d) & 1)
#endif
                          )) {
                if (j1 > 0) {
                    b = lshift(b, 1);
                    j1 = cmp(b, S);
                    if ((j1 > 0 || (j1 == 0 && dig & 1))
                        && dig++ == '9')
                        goto round_9_up;
                }
                *s++ = dig;
                goto ret;
            }
            if (j1 > 0) {
                if (dig == '9') { /* possible if i == 1 */
                round_9_up:
                    *s++ = '9';
                    goto roundoff;
                }
                *s++ = dig + 1;
                goto ret;
            }
            *s++ = dig;
            if (i == ilim)
                break;
            b = multadd(b, 10, 0);
            if (mlo == mhi)
                mlo = mhi = multadd(mhi, 10, 0);
            else {
                mlo = multadd(mlo, 10, 0);
                mhi = multadd(mhi, 10, 0);
            }
        }
    }
    else
        for(i = 1;; i++) {
            *s++ = dig = quorem(b,S) + '0';
            if (i >= ilim)
                break;
            b = multadd(b, 10, 0);
        }

    /* Round off last digit */

    b = lshift(b, 1);
    j = cmp(b, S);
    if (j > 0 || (j == 0 && dig & 1)) {
    roundoff:
        while(*--s == '9')
            if (s == s0) {
                k++;
                *s++ = '1';
                goto ret;
            }
        ++*s++;
    }
    else {
        while(*--s == '0');
        s++;
    }
 ret:
    Bfree(S);
    if (mhi) {
        if (mlo && mlo != mhi)
            Bfree(mlo);
        Bfree(mhi);
    }
 ret1:
    Bfree(b);
    if (s == s0) {                                /* don't return empty string */
        *s++ = '0';
        k = 0;
    }
    *s = 0;
    *decpt = k + 1;
    if (rve)
        *rve = s;
    return s0;
}

#endif // QT_QLOCALE_USES_FCVT
