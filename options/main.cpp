#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcstring.h>
#include <qapplication.h>

#include "options.h"

#include <qmap.h>
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

//#define FIRST_TEST
//#define SECOND_TEST

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);

	Options o;

#ifdef FIRST_TEST

	o.setProperty("/toolbars/blah/1", QVariant("blah"));
	o.setProperty("/toolbars/blah/2", QVariant("blah"));
	o.setProperty("/toolbars/blah/3", QVariant("blah"));
	o.setProperty("/test", QVariant("blah"));
	o.setProperty("/blah", QVariant("blah"));
	o.setProperty("/toolbars/width", QVariant("blah"));
	o.setProperty("/toolbars/height", QVariant("blah"));
	o.setProperty("/toolbars/weight", QVariant("blah"));
	//qWarning("test = \"%s\"", o.property("test").toString().latin1());

#elif SECOND_TEST

	QString string = "string";
	o.setProperty("/string", QVariant(string));

	QStringList strlist;
	strlist << "item1";
	strlist << "item2";
	strlist << "item3";
	o.setProperty("/string_list", QVariant(strlist));

	QPixmap pixmap;
	o.setProperty("/pixmap", QVariant(pixmap));

	QRect rect(10, 20, 50, 60);
	o.setProperty("/rect", QVariant(rect));

	QSize size(100, 200);
	o.setProperty("/size", QVariant(size));

	QColor color(10, 20, 30);
	o.setProperty("/color", QVariant(color));

	QPoint point(70, 80);
	o.setProperty("/point", QVariant(point));

	QImage image;
	o.setProperty("/image", QVariant(image));

	int integer = 666;
	o.setProperty("/int", QVariant(integer));

	uint uinteger = 666666;
	o.setProperty("/uint", QVariant(uinteger));

	bool boolean = true;
	o.setProperty("/bool", QVariant(boolean, 0));

	double dbl = 9.882;
	o.setProperty("/double", QVariant(dbl));

	QDate date;
	o.setProperty("/date", QVariant(date));

	QTime time;
	o.setProperty("/time", QVariant(time));

	QDateTime datetime;
	o.setProperty("/datetime", QVariant(datetime));

	QByteArray ba(15);
	o.setProperty("/bytearray", QVariant(ba));

	QKeySequence keyseq;
	o.setProperty("/keyseq", QVariant(keyseq));

	QMap<QString, QVariant> map;
	map["blah"] = "blah-blah";
	map["kewl"] = "test";
	o.setProperty("/map", QVariant(map));

	QFont font;
	o.setProperty("/font", QVariant(font));

	QBrush brush;
	o.setProperty("/brush", QVariant(brush));

	QPalette pal;
	o.setProperty("/palette", QVariant(pal));

	QPointArray parr;
	o.setProperty("/point_array", QVariant(parr));

	QRegion region;
	o.setProperty("/region", QVariant(region));

	QBitmap bitmap;
	o.setProperty("/bitmap", QVariant(bitmap));

	QCursor cursor;
	o.setProperty("/cursor", QVariant(cursor));

	QBitArray bitarr;
	o.setProperty("/bit_arr", QVariant(bitarr));

	QPen pen;
	o.setProperty("/pen", QVariant(pen));

	QSizePolicy sizep;
	o.setProperty("/size_pol", QVariant(sizep));

	QDomDocument doc;
	doc.appendChild( o.toXml( &doc ) );
	qWarning ( " --- " );
	qWarning ( "%s", doc.toString(8).latin1() );

#else

	QFile file ( "options.xml" );
	file.open (IO_ReadOnly);
	QDomDocument doc;
	if ( doc.setContent(&file, false) )
		o.fromXml( doc.documentElement() );
	else
		qWarning("main: badXML");

#endif

	// ---
	QFile file2 ( "options-2.xml" );
	file2.open ( IO_WriteOnly );
	QTextStream out ( &file2 );
	out.setEncoding(QTextStream::UnicodeUTF8);

	QDomDocument doc2;
	doc2.appendChild( o.toXml ( &doc2 ) );

	out << doc2.toString(8);

	return 0;
}

