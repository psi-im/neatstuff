#include <qapplication.h>
#include <qlayout.h>
#include <qtextview.h>
#include "lineedit.h"

int main( int argc, char *argv[] )
{
	QApplication app( argc, argv );

	QWidget w;
	app.setMainWidget( &w );

	QVBoxLayout l( &w );

	QTextView tv( &w );
	tv.setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
	l.addWidget( &tv );

	LineEdit le( &w );
	l.addWidget( &le );

	w.show();
	return app.exec();
}
