#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcstring.h>
#include <qapplication.h>

#include "vcard.h"
#include "vcardfactory.h"

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);

	VCard vcard;
	const VCard *vc2 = VCardFactory::vcard("test@test.com");
	if ( vc2 )
		vcard = *vc2;

	QFile file2 ( "vcard-out.xml" );
	file2.open ( IO_WriteOnly );
	QTextStream out ( &file2 );

	QDomDocument doc2;
	doc2.appendChild( vcard.toXml ( &doc2 ) );

	out << doc2.toString(8);

	if ( vcard.isEmpty() )
		qWarning("VCard is empty!!!");

	qWarning ( "%s", doc2.toString(8).latin1() );*/

	return 0;
}

