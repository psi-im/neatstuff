#include <qapplication.h>
#include <qlayout.h>
#include <qtextview.h>
#include "lineedit.h"

int main( int argc, char *argv[] )
{
	QApplication app( argc, argv );

	QWidget w( 0, 0, QWidget::WRepaintNoErase );
	app.setMainWidget( &w );

	QVBoxLayout l( &w );

	QTextView tv( &w );
	tv.setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
	l.addWidget( &tv );

	LineEdit le( &w );
	l.addWidget( &le );

	le.setFocus();

	QObject::connect( &le, SIGNAL( addText( const QString & ) ), &tv, SLOT( append( const QString & ) ) );

	w.show();
	return app.exec();
}
