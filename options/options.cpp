/****************************************************************************
** options.cpp - Options' handling class
** Copyright (C) 2003  Michail Pishchagin
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,USA.
**
****************************************************************************/

#include "options.h"

#include "base64.h"

#include <qmap.h>
#include <qdom.h>

#include <qpixmap.h>
#include <qimage.h>
#include <qrect.h>
#include <qsize.h>
#include <qpoint.h>
#include <qdatetime.h>
#include <qkeysequence.h>
#include <qfont.h>
#include <qbrush.h>
#include <qpen.h>
#include <qpalette.h>
#include <qpointarray.h>
#include <qregion.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qbitarray.h>
#include <qsizepolicy.h>

// TODO: need commonxml.h :)

static QDomElement textTag(QDomDocument *doc, const QString &name, const QString &content)
{
	QDomElement tag = doc->createElement(name);
	QDomText text = doc->createTextNode(content);
	tag.appendChild(text);

	return tag;
}

static QDomElement textTag(QDomDocument *doc, const QString &name, const int i)
{
	return textTag(doc, name, QString("%1").arg(i));
}

static QDomElement findSubTag(const QDomElement &e, const QString &name, bool *found)
{
	if(found)
		*found = FALSE;

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		if(i.tagName().upper() == name.upper()) { // mblsha: ignore case when searching
			if(found)
				*found = TRUE;
			return i;
		}
	}

	QDomElement tmp;
	return tmp;
}

// mblsha's own functions

static QDomElement emptyTag(QDomDocument *doc, const QString &name)
{
	QDomElement tag = doc->createElement(name);

	return tag;
}

static bool hasSubTag(const QDomElement &e, const QString &name)
{
	bool found;
	findSubTag(e, name, &found);
	return found;
}

static QString subTagText(const QDomElement &e, const QString &name)
{
	bool found;
	QDomElement i = findSubTag(e, name, &found);
	if ( found )
		return i.text();
	return QString::null;
}

static int subTagInt(const QDomElement &e, const QString &name)
{
	bool found;
	int num = 0;
	QDomElement i = findSubTag(e, name, &found);
	if ( found )
		num = i.text().toInt();
	return num;
}

static uint subTagUInt(const QDomElement &e, const QString &name)
{
	bool found;
	uint num = 0;
	QDomElement i = findSubTag(e, name, &found);
	if ( found )
		num = i.text().toUInt();
	return num;
}

static double subTagDouble(const QDomElement &e, const QString &name)
{
	bool found;
	double num = 0;
	QDomElement i = findSubTag(e, name, &found);
	if ( found )
		num = i.text().toDouble();
	return num;
}

static bool subTagBool(const QDomElement &e, const QString &name)
{
	bool found;
	bool num = false;
	QDomElement i = findSubTag(e, name, &found);
	if ( found )
		num = (i.text() == "true") ? true : false;
	return num;
}

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------

/*!
	\class Options
	\brief Generic class for handling Options

	This class allows you to assign symbolic names to your options and it
	supports import/export to/from XML. All options are stored as QVariant values.

	<i>How to properly name properties:</i>
	-# ALL names should begin from leading slash. Examples:
		- \c "/lookfeel/style"
		- \c "/lookwindows_version"
	-# Valid symbols to use in names: [a-z], [A-Z], [0-9], '_', '-'.
	   In general, all letters, that can be used as XML tag names, can be used freely.
	-# It is possible to combine multiple properties in groups, but there MUST NOT be property
	   with the name of the group. Examples: \n
	   \b VALID:
	   \verbatim
 "/lookfeel/style"    = "WindowsXP"
 "/lookfeel/numFonts" = "4"
 "/lookfeel/blah"     = "blah"
	   \endverbatim
	   \b INVALID:
	   \verbatim
 "/lookfeel/style"    = "WindowsXP"
 "/lookfeel/numFonts" = "4"
 "/lookfeel"          = "something" <-- This is WRONG
	   \endverbatim

	<i>Example of imaginary property tree:</i>
	- \c "/test" = \c "test"
	- \c "/lookfeel/style" = \c "WindowsXP"
	- \c "/lookfeel/numFonts" = \c "64"
	- \c "/lookfeel/nested/something" = \c "some_value"
	- \c "/lookfeel/nested/something_different" = \c "some_other_value"
	- \c "/lookfeel/blah" = \c "blah-blah-blah"
*/

//! \if _hide_doc_
class Options::Private
{
public:
	Private() { }

	void saveProps(QDomDocument *doc, QDomElement &e, QStringList keys, QString parent);
	void saveItem(QDomDocument *doc, QDomElement &parent, const QVariant &var);

	QString formatVersion;
	void loadProps(const QDomElement &e, const QString &parent);
	QVariant loadItem(const QDomElement &e);

	typedef QMap<QString, QVariant> PropMap;
	PropMap props;
};
//! \endif

//!
//! Constructs object.
Options::Options()
{
	d = new Private;
}

//!
//! Destroys object and frees allocated resources.
Options::~Options()
{
	delete d;
}

//!
//! Copies all properties from \a o.
Options &Options::operator=(const Options &o)
{
	d->props = o.d->props;
	return *this;
}

//!
//! Returns property if \a key was found, or empty QVariant if not.
QVariant Options::property(const QString &key) const
{
	Private::PropMap::Iterator it = d->props.find(key);
	if ( it != d->props.end() )
		return d->props[key];
	return QVariant();
}

//!
//! Sets the property with the key \a key to value \a val.
void Options::setProperty(const QString &key, const QVariant &val)
{
	d->props[key] = val;
}

//!
//! Returns \c true if property with the key \a key exists, or false otherwise.
bool Options::exists(const QString &key)
{
	Private::PropMap::Iterator it = d->props.find(key);
	return it != d->props.end();
}

void Options::Private::saveItem(QDomDocument *doc, QDomElement &p, const QVariant &var)
{
	QString type  = QString::null;
	QString value = QString::null;

	switch ( var.type() ) {
		case QVariant::Invalid:
		case QVariant::ColorGroup:
		case QVariant::IconSet:
			qWarning ("WARNING! Options::toXml: unsupported type: %d", var.type());
			break;

		case QVariant::CString:
		case QVariant::String:
			type  = "string";
			value = var.toString();
			break;

		case QVariant::StringList:
			type = "string_list";
			{
				QStringList keys = var.toStringList();
				QStringList::Iterator it = keys.begin();
				for ( ; it != keys.end(); ++it)
					p.appendChild( textTag(doc, "item", *it) );
			}
			break;

		case QVariant::Pixmap:
			type = "pixmap";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toPixmap();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Rect:
			type = "rect";
			{
				QRect r = var.toRect();
				p.appendChild( textTag(doc, "x",	r.x()) );
				p.appendChild( textTag(doc, "y",	r.y()) );
				p.appendChild( textTag(doc, "width",	r.width()) );
				p.appendChild( textTag(doc, "height",	r.height()) );
			}
			break;

		case QVariant::Size:
			type = "size";
			{
				QSize s = var.toSize();
				p.appendChild( textTag(doc, "width",	s.width()) );
				p.appendChild( textTag(doc, "height",	s.height()) );
			}
			break;

		case QVariant::Color:
			type = "color";
			value = var.toColor().name();
			break;

		case QVariant::Point:
			type = "point";
			{
				QPoint t = var.toPoint();
				p.appendChild( textTag(doc, "x", t.x()) );
				p.appendChild( textTag(doc, "y", t.y()) );
			}
			break;

		case QVariant::Image:
			type = "image";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toImage();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Int:
			type = "int";
			value = QString("%1").arg( var.toInt() );
			break;

		case QVariant::UInt:
			type = "uint";
			value = QString("%1").arg( var.toUInt() );
			break;

		case QVariant::Bool:
			type = "bool";
			if ( var.toBool() )
				value = "true";
			else
				value = "false";
			break;

		case QVariant::Double:
			type = "double";
			value = QString("%1").arg( var.toDouble() );
			break;

		case QVariant::Date:
			type = "date";
			value = var.toDate().toString(Qt::ISODate);
			break;

		case QVariant::Time:
			type = "time";
			value = var.toTime().toString();
			break;

		case QVariant::DateTime:
			type = "date_time";
			value = var.toDateTime().toString(Qt::ISODate);
			break;

		case QVariant::ByteArray:
			type = "byte_array";
			value = Base64::arrayToString( var.toByteArray() );
			break;

		case QVariant::KeySequence:
			type = "key_sequence";
			value = (QString)var.toKeySequence();
			break;

		case QVariant::Map:
			type = "map";
			{
				QMap<QString, QVariant> map = var.toMap();
				QMap<QString, QVariant>::Iterator it = map.begin();
				for ( ; it != map.end(); ++it) {
					QDomElement e = doc->createElement( "item" );
					e.appendChild( textTag(doc, "key", it.key()) );

					QDomElement v = doc->createElement( "data" );
					saveItem (doc, v, it.data());
					e.appendChild(v);

					p.appendChild(e);
				}
			}
			break;

		case QVariant::Font:
			type = "font";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toFont();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Brush:
			type = "brush";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toBrush();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Palette:
			type = "palette";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toPalette();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::PointArray:
			type = "point_array";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toPointArray();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Region:
			type = "region";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toRegion();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Bitmap:
			type = "bitmap";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toBitmap();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Cursor:
			type = "cursor";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toCursor();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::BitArray:
			type = "bit_array";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toBitArray();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::Pen:
			type = "pen";
			{
				QByteArray ba;
				QDataStream stream (ba, IO_WriteOnly);
				stream << var.toPen();
				value = Base64::arrayToString(ba);
			}
			break;

		case QVariant::SizePolicy:
			type = "size_policy";
			{
				QSizePolicy s = var.toSizePolicy();
				p.appendChild( textTag(doc, "hor",		s.horData()) );
				p.appendChild( textTag(doc, "ver",		s.verData()) );
				p.appendChild( textTag(doc, "horStretch",	s.horStretch()) );
				p.appendChild( textTag(doc, "verStretch",	s.verStretch()) );
				p.appendChild( textTag(doc, "hfw",		s.hasHeightForWidth() ? "true" : "false") );
			}
			break;

		case QVariant::List:
			type = "list";
			{
				QValueList<QVariant> list = var.toList();
				QValueList<QVariant>::Iterator it = list.begin();
				for ( ; it != list.end(); ++it) {
					QDomElement e = doc->createElement( "item" );
					saveItem (doc, e, *it);
					p.appendChild(e);
				}
			}
			break;
	}

	if ( !type.isEmpty() ) {
		p.setAttribute("type", type);

		if ( !value.isEmpty() ) {
			QDomText text = doc->createTextNode( value );
			p.appendChild(text);
		}
	}
}

void Options::Private::saveProps(QDomDocument *doc, QDomElement &e, QStringList keys, QString parent)
{
	QStringList savedKeys;

	QStringList::Iterator it = keys.begin();
	for ( ; it != keys.end(); ++it) {
		int count = 0;
		int slash = (*it).find( '/' );
		if ( slash != -1 ) {
			QString name = (*it).left( slash );
			if ( !name.isEmpty() && (savedKeys.find( name ) == savedKeys.end()) ) {
				QStringList::Iterator it2 = keys.begin();
				for ( ; it2 != keys.end(); ++it2)
					if ( (*it2).left( slash ) == name )
						count++;

				if ( count > 1 ) {
					QStringList newKeys;
					it2 = keys.begin();
					for ( ; it2 != keys.end(); ++it2)
						if ( (*it2).left( slash ) == name )
							newKeys << (*it2).mid( slash + 1 );

					QDomElement p = doc->createElement(name);
					saveProps(doc, p, newKeys, parent + "/" + name);
					e.appendChild(p);

					savedKeys << name;
				}
			}
		}

		if ( slash == -1 || count == 1 ) {
			QDomElement p = doc->createElement( *it );
			saveItem (doc, p, props[parent + "/" + (*it).latin1()]);
			e.appendChild(p);
		}
	}
}

/*!
	Creates \c QDomElement from \c QDomDocument \a doc with all properties inside it.
	Use it to save Options to file, send it over network, etc.

	\code
 Options o;

 // ...

 QFile file ( "options.xml" );
 file.open ( IO_WriteOnly );
 QTextStream out ( &file );
 out.setEncoding ( QTextStream::UnicodeUTF8 );

 QDomDocument doc;
 doc.appendChild ( o.toXml ( &doc ) );

 out << doc.toString(4);
	\endcode

	\sa fromXml()
*/
QDomElement Options::toXml(QDomDocument *doc) const
{
	QDomElement o = doc->createElement("options");
	o.setAttribute("version", "1.0");

	QStringList keys;
	Private::PropMap::Iterator pit = d->props.begin();
	for ( ; pit != d->props.end(); ++pit)
		keys << pit.key();
	keys.sort();

	// strip leading slash
	QStringList k;
	QStringList::Iterator it = keys.begin();
	for ( ; it != keys.end(); ++it) {
		int index = ((*it).at(0) == '/') ? 1 : 0;
		k << (*it).mid( index );

		if ( !index )
			qDebug("ACHTUNG! Options::toXml: Missing slash in front of \"%s\" option.", (*it).latin1());
	}

	d->saveProps(doc, o, k, "");

	return o;
}

QVariant Options::Private::loadItem(const QDomElement &i)
{
	QVariant v;

	QString type = i.attribute("type");

	if ( type == "string" ) {
		v = QVariant( i.text() );
	}
	else if ( type == "string_list" ) {
		QStringList list;

		QDomNode n = i.firstChild();
		for ( ; !n.isNull(); n = n.nextSibling() ) {
			QDomElement e = n.toElement();
			if ( e.isNull() || e.tagName() != "item" )
				continue;

			list << e.text();
		}

		v = QVariant( list );
	}
	else if ( type == "pixmap" ) {
		QPixmap pix;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> pix;

		v = QVariant( pix );
	}
	else if ( type == "rect" ) {
		QRect r;

		r.setX		( subTagInt(i, "x") );
		r.setY		( subTagInt(i, "y") );
		r.setWidth	( subTagInt(i, "width") );
		r.setHeight	( subTagInt(i, "height") );

		v = QVariant( r );
	}
	else if ( type == "size" ) {
		QSize s;

		s.setWidth	( subTagInt(i, "width") );
		s.setHeight	( subTagInt(i, "height") );

		v = QVariant( s );
	}
	else if ( type == "color" ) {
		QColor col( i.text() );
		v = QVariant( col );
	}
	else if ( type == "point" ) {
		QPoint p;

		p.setX ( subTagInt(i, "x") );
		p.setY ( subTagInt(i, "y") );

		v = QVariant( p );
	}
	else if ( type == "image" ) {
		QImage img;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> img;

		v = QVariant( img );
	}
	else if ( type == "int" ) {
		int integer;

		integer = i.text().toInt();

		v = QVariant( integer );
	}
	else if ( type == "uint" ) {
		uint integer;

		integer = i.text().toUInt();

		v = QVariant( integer );
	}
	else if ( type == "bool" ) {
		bool boolean;

		boolean = (i.text() == "true") ? true : false;

		v = QVariant( boolean, 0 );
	}
	else if ( type == "double" ) {
		double dbl;

		dbl = i.text().toDouble();

		v = QVariant( dbl );
	}
	else if ( type == "date" ) {
		QDate date;

		date = QDate::fromString( i.text(), Qt::ISODate );

		v = QVariant( date );
	}
	else if ( type == "time" ) {
		QTime time;

		time = QTime::fromString( i.text() );

		v = QVariant( time );
	}
	else if ( type == "date_time" ) {
		QDateTime datetime;

		datetime = QDateTime::fromString( i.text(), Qt::ISODate );

		v = QVariant( datetime );
	}
	else if ( type == "byte_array" ) {
		QByteArray ba = Base64::stringToArray( i.text() );

		v = QVariant( ba );
	}
	else if ( type == "key_sequence") {
		QKeySequence keyseq( i.text() );

		v = QVariant( keyseq );
	}
	else if ( type == "map" ) {
		QMap<QString, QVariant> map;

		QDomNode n = i.firstChild();
		for ( ; !n.isNull(); n = n.nextSibling() ) {
			QDomElement e = n.toElement();
			if ( e.isNull() || e.tagName() != "item" )
				continue;

			QDomElement data = findSubTag(e, "data", 0);
			map[ subTagText(e, "key") ] = loadItem(data);
		}

		v = QVariant( map );
	}
	else if ( type == "font" ) {
		QFont font;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> font;

		v = QVariant( font );
	}
	else if ( type == "brush" ) {
		QBrush brush;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> brush;

		v = QVariant( brush );
	}
	else if ( type == "palette" ) {
		QPalette pal;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> pal;

		v = QVariant( pal );
	}
	else if ( type == "point_array" ) {
		QPointArray pa;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> pa;

		v = QVariant( pa );
	}
	else if ( type == "region" ) {
		QRegion region;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> region;

		v = QVariant( region );
	}
	else if ( type == "bitmap" ) {
		QBitmap bm;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> bm;

		v = QVariant( bm );
	}
	else if ( type == "cursor" ) {
		QCursor cur;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> cur;

		v = QVariant( cur );
	}
	else if ( type == "bit_array" ) {
		QBitArray bita;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> bita;

		v = QVariant( bita );
	}
	else if ( type == "pen" ) {
		QPen pen;

		QByteArray ba = Base64::stringToArray( i.text() );
		QDataStream stream (ba, IO_ReadOnly);
		stream >> pen;

		v = QVariant( pen );
	}
	else if ( type == "size_policy" ) {
		QSizePolicy sp( (QSizePolicy::SizeType)subTagInt(i, "hor"),
				(QSizePolicy::SizeType)subTagInt(i, "ver"),
				subTagInt(i, "horStretch"),
				subTagInt(i, "verStretch"),
				subTagBool(i, "hfw") );

		v = QVariant( sp );
	}
	else if ( type == "list" ) {
		QValueList<QVariant> list;

		QDomNode n = i.firstChild();
		for ( ; !n.isNull(); n = n.nextSibling() ) {
			QDomElement e = n.toElement();
			if ( e.isNull() || e.tagName() != "item" )
				continue;

			list.append( loadItem(e) );
		}

		v = QVariant( list );
	}
	else {
		qDebug("WARNING! Options::fromXml: unknown type '%s'", type.latin1());
	}

	return v;
}

void Options::Private::loadProps(const QDomElement &q, const QString &parent)
{
	QDomNode n = q.firstChild();
	for ( ; !n.isNull(); n = n.nextSibling() ) {
		QDomElement i = n.toElement();
		if ( i.isNull() )
			continue;

		if ( i.attribute("type").isEmpty() )
			loadProps( i, parent + "/" + i.tagName() );
		else
			props[parent + "/" + i.tagName()] = loadItem(i);
	}
}

/*!
	Reads properties from \c QDomElement \a q. Use it to restore saved options from file.

	\code
 Options o;

 QFile file ( "options.xml" );
 file.open ( IO_ReadOnly );
 QDomDocument doc;
 doc.setContent( &file, false );
 o.fromXml( doc.documentElement() );
	\endcode

	\sa toXml()
*/
bool Options::fromXml(const QDomElement &q)
{
	if ( q.tagName() != "options" )
		return false;

	d->formatVersion = q.attribute("version");
	d->loadProps( q, "" );

	return true;
}

//!
//! Returns \c true if there at least one property and \c false otherwise.
bool Options::isEmpty() const
{
	return d->props.isEmpty();
}
