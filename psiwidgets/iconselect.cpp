#include "iconselect.h"

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qbutton.h>

#include <qlabel.h>
#include <qtooltip.h>

#include <math.h>

#include "iconset.h"

//----------------------------------------------------------------------------
// IconSelectButton
//----------------------------------------------------------------------------

class IconSelectButton : public QButton
{
	Q_OBJECT

private:
	Icon *ic;
	QSize s;
	bool animated;

public:
	IconSelectButton(QWidget *parent, const char *name = 0)
	: QButton(parent, name, WRepaintNoErase)
	{
		//setFocusPolicy(StrongFocus);
		ic = 0;
		animated = false;
		connect (this, SIGNAL(clicked()), SLOT(iconSelected()));
	}

	~IconSelectButton()
	{
		iconStop();
	}

	void setIcon(const Icon *i)
	{
		iconStop();
		ic = (Icon *)i;
		//iconStart();
	}

	const Icon *icon() const
	{
		return ic;
	}

	QSize sizeHint() const { return s; }
	void setSizeHint(QSize sh) { s = sh; }

signals:
	void iconSelected(const Icon *);

public slots:
	void aboutToShow() { iconStart(); }
	void aboutToHide() { iconStop();  }

private:
	void iconStart()
	{
		if ( ic ) {
			connect(ic, SIGNAL(pixmapChanged(const QPixmap &)), SLOT(iconUpdated(const QPixmap &)));
			if ( !animated ) {
				ic->activated(false);
				animated = true;
			}

			if ( !ic->text().isEmpty() ) {
				QString str;
				QDictIterator<QString> it ( ic->text() );
				/*for ( ; it.current(); ++it) {
					if ( !str.isEmpty() )
						str += ", ";
					str += **it;
				}*/
				str = **it;
				QToolTip::add(this, str);
			}
		}
	}

	void iconStop()
	{
		if ( ic ) {
			disconnect(ic, 0, this, 0 );
			if ( animated ) {
				ic->stop();
				animated = false;
			}
		}
	}

	void enterEvent(QEvent *) { setFocus(); } // focus follows mouse mode

private slots:
	void iconUpdated(const QPixmap &)
	{
		update();
	}

	void iconSelected()
	{
		clearFocus();
		if ( ic ) {
			emit iconSelected(ic);
			parentWidget()->parentWidget()->close();
		}
	}

private:
	void drawButton (QPainter *p)
	{
		int flags = QStyle::Style_Enabled;

		if ( hasFocus() /*|| hasMouse()*/ /*|| isDown() || isOn()*/ )
			flags |= QStyle::Style_Active;

		QMenuItem dummy;
		style().drawControl(QStyle::CE_PopupMenuItem, p, this, rect(), colorGroup(),
				flags, QStyleOption(&dummy, 0, 0));

		if ( ic ) {
			QPixmap pix = ic->pixmap();
			p->drawPixmap((width() - pix.width())/2, (height() - pix.height())/2, pix);
		}
	}
};

//----------------------------------------------------------------------------
// IconSelect -- the widget that does all dirty work
//----------------------------------------------------------------------------

class IconSelect : public QWidget
{
	Q_OBJECT

private:
	Iconset is;
	QGridLayout *grid;

public:
	IconSelect(QWidget *parent, const char *name = 0);
	~IconSelect();

	void setIconset(const Iconset &);
	const Iconset &iconset() const;

	//bool isFocusEnabled () const { return true; }
	//void keyPressEvent(QKeyEvent *);

	/*void paintEvent(QPaintEvent *e)
	{
		QWidget *pw = (QWidget *)parent();
		if ( !pw || !pw->paletteBackgroundPixmap() )
			return;

		QPainter paint;
		paint.begin(this);

		QRect r = rect();
		QPixmap pix(r.width(), r.height());
		QPainter p;
		p.begin(&pix);
		p.drawTiledPixmap (r, *pw->paletteBackgroundPixmap(), mapToParent( r.topLeft() ));
		//QLabel::drawContents ( &p );
		p.end();

		paint.drawPixmap (r.topLeft(), pix );

		paint.end();
	}*/

	void noIcons();
};

IconSelect::IconSelect(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	grid = 0;
	noIcons();
	setFocus();
}

IconSelect::~IconSelect()
{
}

void IconSelect::noIcons()
{
	grid = new QGridLayout(this);
	grid->setAutoAdd(true);

	QLabel *lbl = new QLabel(this);
	lbl->setText( tr("No icons available") );
}

void IconSelect::setIconset(const Iconset &iconset)
{
	is = iconset;

	// delete all children
	if (grid) {
		delete grid;

		QObjectList *l = queryList();
		l->setAutoDelete(true);
		delete l;
	}

	if ( !is.count() ) {
		noIcons();
		return;
	}

	// first we need to find optimal size for elements and don't forget about
	// taking too much screen space
	float w = 0, h = 0;

	int count;
	QDictIterator<Icon> it = is.iterator();
	for (count = 0; it.current(); ++it, count++) {
		w += (*it)->pixmap().width();
		h += (*it)->pixmap().height();
	}
	w /= count;
	h /= count;

	const int margin = 2;
	int tileSize = (int)QMAX(w, h) + 2*margin;

	QRect r = QApplication::desktop()->availableGeometry( this );
	int maxSize = QMIN(r.width(), r.height())/2;

	int size = (int)ceil( sqrt( count ) );

	if ( size*tileSize > maxSize ) { // too many icons. find reasonable size.
		int c = 0;
		for (w = 0; w <= maxSize; w += tileSize)
			c++;
		size = c - 1;
	}

	// now, fill grid with elements
	grid = new QGridLayout(this, size, size);
	grid->setAutoAdd(true);

	count = 0;

	it = is.iterator();
	for ( ; it.current(); ++it) {
		if ( ++count > size*size )
			break;

		IconSelectButton *b = new IconSelectButton(this);
		b->setIcon( *it );
		b->setSizeHint( QSize(tileSize, tileSize) );
		connect (b, SIGNAL(iconSelected(const Icon *)), parent(), SIGNAL(iconSelected(const Icon *)));

		connect (parent(), SIGNAL(aboutToShow()), b, SLOT(aboutToShow()));
		connect (parent(), SIGNAL(aboutToHide()), b, SLOT(aboutToHide()));
	}
}

const Iconset &IconSelect::iconset() const
{
	return is;
}

/*void IconSelect::keyPressEvent(QKeyEvent *e)
{
	int it = 0;
	switch ( e->key() ) {
		case Key_Tab:
			it = e->state() & ShiftButton ? -1 : 1;
			break;

		case Key_Up:
		case Key_Left:
			it = -1;
			break;

		case Key_Down:
		case Key_Right:
			it = 1;
			break;

		default:
			e->ignore();
	}

	if ( it )
		focusNextPrevChild ( it > 0 );
}*/

//----------------------------------------------------------------------------
// IconSelectPopup
//----------------------------------------------------------------------------

class IconSelectPopup::Private : public QObject
{
public:
	Private(QWidget *parent)
	: QObject(parent)
	{
		icsel = new IconSelect(parent);
	}

	IconSelect *icsel;
};

IconSelectPopup::IconSelectPopup(QWidget *parent, const char *name)
: QPopupMenu(parent, name)
{
	d = new Private(this);

	insertItem (d->icsel);
}

IconSelectPopup::~IconSelectPopup()
{
}

void IconSelectPopup::setIconset(const Iconset &i)
{
	d->icsel->setIconset(i);
}

const Iconset &IconSelectPopup::iconset() const
{
	return d->icsel->iconset();
}

#include "iconselect.moc"
