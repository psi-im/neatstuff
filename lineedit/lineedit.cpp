#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include "lineedit.h"

LineEdit::LineEdit(QWidget *parent)
	: QTextEdit(parent)
{
	setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );

	setVScrollBarMode(QScrollView::AlwaysOff);
	setHScrollBarMode(QScrollView::AlwaysOff);

	setTextFormat(RichText);

	connect( this, SIGNAL( textChanged() ), SLOT( recalculateSize() ) );
}

LineEdit::~LineEdit()
{
}

QSize LineEdit::sizeHint() const
{
	QSize s( QTextEdit::sizeHint().width(), 0 );
	int h = 5;

	if ( paragraphs() > 0 ) {
		for ( int i = 0; i < paragraphs(); i++ ) {
			h += paragraphRect( i ).height();
		}
	}
	else
		h += heightForWidth( s.width() );

	if ( horizontalScrollBar()->isVisible() )
		h += horizontalScrollBar()->height();

	QRect desktop = qApp->desktop()->availableGeometry( this );


	s.setHeight( h );
	return s;
}

void LineEdit::recalculateSize()
{
	QSize oldSize = size();
	QSize newSize = sizeHint();

	if ( newSize.height() != oldSize.height() ) {
		parentWidget()->resize( parentWidget()->width(), parentWidget()->height() + ( newSize.height() - oldSize.height() ) );
		parentWidget()->layout()->activate();
	}
}
