#include <qapplication.h>
#include <qdom.h>
#include <qfile.h>

#include "xdata_widget.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QFile file ( "form2.xml" );
	file.open (IO_ReadOnly);
	QDomDocument doc;
	if ( !doc.setContent(&file, false) ) {
		qWarning("Unable to doc::setContext()!!!");
		return 2;
	}

	XData xdata;
	xdata.fromXml( doc.documentElement() );

	// widget...
	XDataWidget *widget = new XDataWidget();
	app.setMainWidget(widget);

	widget->setFields( xdata.fields() );
	widget->show();

	int ret = app.exec();

	xdata.setFields( widget->fields() );

	QDomDocument doc2;
	doc2.appendChild( xdata.toXml ( &doc2 ) );
	//out << doc2.toString(8);
	qWarning ( "%s", doc2.toString(8).latin1() );

	return ret;
}
