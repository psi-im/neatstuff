#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include "lineedit.h"

class LineEdit::Private
{
public:
	QWidget *topParent;
	QSize lastSize;
};

LineEdit::LineEdit(QWidget *parent)
	: QTextEdit(parent)
{
	d = new Private;
	d->topParent = topLevelWidget();
	d->lastSize = QSize( 0, 0 );

	// LineEdit's size hint is to be as small as possible
	setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );

#ifdef Q_WS_MAC
	setVScrollBarMode(QScrollView::AlwaysOff);
	setHScrollBarMode(QScrollView::AlwaysOff);
#endif

	setWordWrap(QTextEdit::WidgetWidth);

	setReadOnly(false);
	setUndoRedoEnabled(true);

	setTextFormat(PlainText);

	connect( this, SIGNAL( textChanged() ), SLOT( recalculateSize() ) );
}

LineEdit::~LineEdit()
{
	delete d;
}

QSize LineEdit::sizeHint() const
{
	QSize s( QTextEdit::sizeHint().width(), 0 );
	int h = 7;

	if ( paragraphs() > 0 ) {
		for ( int i = 0; i < paragraphs(); i++ ) {
			h += paragraphRect( i ).height();
		}
	}

	if ( horizontalScrollBar()->isVisible() )
		h += horizontalScrollBar()->height();

	QRect desktop = qApp->desktop()->availableGeometry( (QWidget *)d->topParent );
	int dh = h - d->lastSize.height();

	// check that our dialog's height doesn't exceed the desktop's
	if ( dh > 0 && (d->topParent->frameGeometry().height() + dh) >= desktop.height() ) {
		h = desktop.height() - ( d->topParent->frameGeometry().height() - size().height() );
	}

	s.setHeight( h );
	return s;
}

void LineEdit::recalculateSize()
{
	QSize oldSize = size();
	QSize newSize = sizeHint();

	if ( newSize.height() != oldSize.height() ) {
		d->lastSize = newSize;

		int dh = newSize.height() - oldSize.height();
		d->topParent->resize( d->topParent->width(),
				      d->topParent->height() + dh );
		d->topParent->layout()->activate();

		// check, if we need to move dialog upper
		QRect desktop = qApp->desktop()->availableGeometry( (QWidget *)d->topParent );
		if ( dh > 0 && ( d->topParent->frameGeometry().bottom() >= desktop.bottom() ) ) {
			int newy = desktop.bottom() - d->topParent->frameGeometry().height();
			d->topParent->move( d->topParent->x(), newy );
		}
	}
}

void LineEdit::keyPressEvent( QKeyEvent *e )
{
	if ( ( e->state() & ControlButton ) && ( e->key() == Key_Enter || e->key() == Key_Return ) ) {
		emit addText( text() );
		clear();

		e->accept();
		return;
	}

	QTextEdit::keyPressEvent( e );
}

