/****************************************************************************
** iconwidget.cpp - misc. widgets for displaying Iconsets
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

#include "iconwidget.h"

#include <qapplication.h>
#include <qpainter.h>

#ifndef WIDGET_PLUGIN
#include "iconset.h"

#include <qstyle.h>
#include <qbitmap.h>
#include <qmap.h>
#include <qtooltip.h>
#endif

//----------------------------------------------------------------------------
// IconsetSelect
//----------------------------------------------------------------------------

class IconsetSelectItem : public IconWidgetItem
{
	Q_OBJECT
private:
	static const int margin = 3;
	static const int displayNumIcons = 10;
#ifndef WIDGET_PLUGIN
	Iconset iss;
	QMap<Icon*, QRect> iconRects;
#endif
	int w, h;
	mutable int fullW, fullH;

public:
	IconsetSelectItem(QListBox *parent, IconsetSelectItem *after, const Iconset &_iconset)
	: IconWidgetItem(parent, after)
	{
#ifndef WIDGET_PLUGIN
		iss = _iconset;
		setText( iss.name() );

		w = margin;
		h = 2*margin;

		int count;

		QDictIterator<Icon> it = iss.iterator();
		for (count = 0; it.current(); ++it) {
			if ( count++ >= displayNumIcons )
				break; // display only first displayNumIcons icons

			QPixmap pix = it.current()->pixmap();

			iconRects[it.current()] = QRect( w, margin, pix.width(), pix.height() );

			w += pix.width() + margin;
			h = QMAX( h, pix.height() + 2*margin );

			connect (it.current(), SIGNAL(pixmapChanged(const QPixmap &)), SLOT(iconUpdated(const QPixmap &)));
			it.current()->activated(false); // start animation
		}

		QMap<Icon*, QRect>::Iterator it2;
		for (it2 = iconRects.begin(); it2 != iconRects.end(); it2++) {
			QRect r = it2.data();
			it2.data() = QRect( r.x(), (h - r.height())/2, r.width(), r.height() );
		}
#endif
	}

	~IconsetSelectItem()
	{
#ifndef WIDGET_PLUGIN
		QMap<Icon*, QRect>::Iterator it;
		for (it = iconRects.begin(); it != iconRects.end(); it++)
			it.key()->stop();
#endif
	}

	const Iconset *iconset() const
	{
#ifndef WIDGET_PLUGIN
		return &iss;
#else
		return 0;
#endif
	}

	int height( const QListBox *lb ) const
	{
		fullH = lb->fontMetrics().lineSpacing() + 2 + h;
		return QMAX( fullH, QApplication::globalStrut().height() );
	}

	int width( const QListBox *lb ) const
	{
		fullW = QMAX(lb->fontMetrics().width( text() ) + 6, w + 10);
		return QMAX( fullW, QApplication::globalStrut().width() );
	}

	void paint(QPainter *painter)
	{
#ifndef WIDGET_PLUGIN
		QFontMetrics fm = painter->fontMetrics();
		painter->drawText( 3, fm.ascent() + (fm.leading()+1)/2 + 1, text() );

		QMap<Icon*, QRect>::Iterator it;
		for (it = iconRects.begin(); it != iconRects.end(); it++) {
			Icon *icon = it.key();
			QRect r = it.data();
			painter->drawPixmap(QPoint(10 + r.left(), fm.lineSpacing() + 2 + r.top()), icon->pixmap());
		}
#endif
	}

private slots:
	void iconUpdated(const QPixmap &)
	{
#ifndef WIDGET_PLUGIN
		IconsetSelect *issel = (IconsetSelect *)listBox();
		issel->updateItem (this);
#endif
	}
};

class IconsetSelect::Private
{
public:
	Private()
	{
		lastItem = 0;
	}

	IconsetSelectItem *lastItem;
};

IconsetSelect::IconsetSelect(QWidget *parent, const char *name)
: QListBox(parent, name)
{
	d = new Private;
}

IconsetSelect::~IconsetSelect()
{
	delete d;
}
	
void IconsetSelect::insert(const Iconset &iconset)
{
#ifndef WIDGET_PLUGIN
	IconsetSelectItem *item = new IconsetSelectItem(this, d->lastItem, iconset);
	d->lastItem = item;
#endif
}

void IconsetSelect::moveItemUp()
{
	if ( currentItem() < 1 )
		return;

	IconsetSelectItem *item = (IconsetSelectItem *)selectedItem();
	if ( !item )
		return;
	QListBoxItem *prev = item->prev()->prev();
	takeItem (item);
	insertItem (item, prev);
	setSelected (item, true);
	setCurrentItem (item);
}

void IconsetSelect::moveItemDown()
{
	if ( currentItem() == -1 || currentItem() > (int)count() - 2 )
		return;

	IconsetSelectItem *item = (IconsetSelectItem *)selectedItem();
	if ( !item )
		return;
	QListBoxItem *next = item->next();
	takeItem (item);
	insertItem (item, next);
	setCurrentItem (item);
}

const Iconset *IconsetSelect::iconset() const
{
	IconsetSelectItem *item = (IconsetSelectItem *)selectedItem();
	if ( item )
		return item->iconset();
	return 0;
}

void IconsetSelect::paintCell(QPainter *p, int row, int col)
{
	// we'll do some caching to avoid flicker
	QListBoxItem *item = QListBox::item(row);

	if ( !item ) {
		QListBox::paintCell(p, row, col);
		return;
	}

	int w = contentsWidth();
	int h = item->height(this);
	QPixmap pix(w, h);
	QPainter p2;
	p2.begin (&pix);
	QListBox::paintCell(&p2, row, col);
	p2.end ();

	p->drawPixmap(QPoint(0, 0), pix);
}


//----------------------------------------------------------------------------
// IconsetDisplay
//----------------------------------------------------------------------------

class IconsetDisplayItem : public IconWidgetItem
{
	Q_OBJECT
private:
	static const int margin = 3;
	Icon *icon;
	int w, h;

public:
	IconsetDisplayItem(QListBox *parent, IconsetDisplayItem *after, Icon *i, int iconW)
	: IconWidgetItem(parent, after)
	{
#ifndef WIDGET_PLUGIN
		icon = i;
		w = iconW;

		connect (icon, SIGNAL(pixmapChanged(const QPixmap &)), SLOT(iconUpdated(const QPixmap &)));
		icon->activated(false);

		h = icon->pixmap().height();

		QString str;
		QDictIterator<QString> it ( icon->text() );
		for ( ; it.current(); ++it) {
			if ( !str.isEmpty() )
				str += ", ";
			str += **it;
		}
		setText(str);
#endif
	}

	~IconsetDisplayItem()
	{
#ifndef WIDGET_PLUGIN
		icon->stop();
#endif
	}

	int height( const QListBox *lb ) const
	{
		int hh = QMAX(h + 2*margin, lb->fontMetrics().lineSpacing() + 2);
		return QMAX( hh, QApplication::globalStrut().height() );
	}

	int width( const QListBox *lb ) const
	{
		int ww = lb->fontMetrics().width( text() ) + 6 + w + 2*margin;
		return QMAX( ww, QApplication::globalStrut().width() );
	}

	void paint(QPainter *painter)
	{
#ifndef WIDGET_PLUGIN
		painter->drawPixmap(QPoint((2*margin+w - icon->pixmap().width())/2, margin), icon->pixmap());
		QFontMetrics fm = painter->fontMetrics();
		//int hh = QMAX(h + 2*margin, fm.lineSpacing() + 2);
		painter->drawText( w + 2*margin + 3, fm.ascent() + (fm.leading()+1)/2 + 1, text() );
		//painter->drawText( w + 2*margin + 3, (hh - fm.lineSpacing())/2, text() );
#endif
	}

private slots:
	void iconUpdated(const QPixmap &)
	{
		IconsetDisplay *issel = (IconsetDisplay *)listBox();
		issel->updateItem (this);
	}
};

class IconsetDisplay::Private
{
public:
	Private()
	{
		lastItem = 0;
	}

	IconsetDisplayItem *lastItem;
};

IconsetDisplay::IconsetDisplay(QWidget *parent, const char *name)
: QListBox(parent, name, WStaticContents | WResizeNoErase | WRepaintNoErase)
{
	d = new Private;
}

IconsetDisplay::~IconsetDisplay()
{
	delete d;
}

void IconsetDisplay::setIconset(const Iconset &iconset)
{
#ifndef WIDGET_PLUGIN
	int w = 0;
	QDictIterator<Icon> it = iconset.iterator();
	for ( ; it.current(); ++it) {
		w = QMAX(w, it.current()->pixmap().width());
	}

	it = iconset.iterator();
	for ( ; it.current(); ++it) {
		IconsetDisplayItem *item = new IconsetDisplayItem(this, d->lastItem, it.current(), w);
		d->lastItem = item;
	}
#endif
}

void IconsetDisplay::paintCell(QPainter *p, int row, int col)
{
	// we'll do some caching to avoid flicker
	QListBoxItem *item = QListBox::item(row);

	if ( !item ) {
		QListBox::paintCell(p, row, col);
		return;
	}

	int w = contentsWidth();
	int h = item->height(this);
	QPixmap pix(w, h);
	QPainter p2;
	p2.begin (&pix);
	QListBox::paintCell(&p2, row, col);
	p2.end ();

	p->drawPixmap(QPoint(0, 0), pix);
}


//----------------------------------------------------------------------------
// IconButton
//----------------------------------------------------------------------------

class IconButton::Private : public QObject
{
	Q_OBJECT
public:
	Icon *icon;
	IconButton *button;
	bool hasToolTip, textVisible;
#ifdef WIDGET_PLUGIN
	QString iconName;
#endif
 
public:
	Private(IconButton *b) 
	{
		icon = 0;
		button = b;
		hasToolTip = false;
		textVisible = true;
	}

	~Private()
	{
		iconStop();
	}

	void setIcon(Icon *i)
	{
		iconStop();
		icon = new Icon(*i);
		iconStart();
	}

	void iconStart()
	{
#ifndef WIDGET_PLUGIN
		if ( icon ) {
			connect(icon, SIGNAL(pixmapChanged(const QPixmap &)), SLOT(iconUpdated(const QPixmap &)));
			icon->activated(true); // FIXME: should icon play sound when it's activated on button?
			iconUpdated( icon->pixmap() );
		}
		else
			iconUpdated( QPixmap() );
#endif
	}

	void iconStop()
	{
#ifndef WIDGET_PLUGIN
		if ( icon ) {
			disconnect(icon, 0, this, 0 );
			icon->stop();

			delete icon;
			icon = 0;
		}
#endif
	}

	void update()
	{
#ifndef WIDGET_PLUGIN
		if ( icon )
			iconUpdated( icon->pixmap() );
#endif
	}

private slots:
	void iconUpdated(const QPixmap &pix)
	{
		button->setUpdatesEnabled(FALSE);
		if ( textVisible )
			button->setIconSet(pix);
		else
			button->setPixmap(pix);
		button->setUpdatesEnabled(TRUE);
		button->update();
	}
};

IconButton::IconButton(QWidget *parent, const char *name)
: QPushButton(parent, name)
{
	setWFlags(getWFlags() | WRepaintNoErase); // no nasty flicker anymore :)
	d = new Private(this);
	
	//setMinimumWidth(48);
	//setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

IconButton::~IconButton()
{
	delete d;
}

void IconButton::setIcon(const Icon *i)
{
	d->setIcon ((Icon *)i);
}

void IconButton::setIcon(const QString &name)
{
#ifndef WIDGET_PLUGIN
	setIcon( IconsetFactory::iconPtr(name) );
#else
	d->iconName = name;
#endif
}

const QString &IconButton::iconName() const
{
#ifndef WIDGET_PLUGIN
	if ( d->icon )
		return d->icon->name();
	return QString::null;
#else
	return d->iconName;
#endif
}

void IconButton::setText(const QString &text)
{
#ifndef WIDGET_PLUGIN
	if ( d->hasToolTip ) {
		QToolTip::remove(this);
		d->hasToolTip = FALSE;
	}
#endif	
	QPushButton::setText( text );
}

bool IconButton::textVisible() const
{
	return d->textVisible;
}

void IconButton::setTextVisible(bool v)
{
	d->textVisible = v;
	d->update();
}

void IconButton::drawButtonLabel(QPainter *p)
{
/*#ifndef WIDGET_PLUGIN
	// crazy code ahead!  watch out for potholes and deer.

	// this gets us the width of the "text area" on the button.
	// adapted from qt/src/styles/qcommonstyle.cpp and qt/src/widgets/qpushbutton.cpp
	QRect r = style().subRect(QStyle::SR_PushButtonContents, this);
	if(isMenuButton())
		r.setWidth(r.width() - style().pixelMetric(QStyle::PM_MenuButtonIndicator, this));
	if(iconSet() && !iconSet()->isNull())
		r.setWidth(r.width() - (iconSet()->pixmap(QIconSet::Small, QIconSet::Normal, QIconSet::Off).width() + 4));

	// font metrics
	QFontMetrics fm(font());

	// w1 = width of button text, w2 = width of text area
	int w1 = fm.width(text());
	int w2 = r.width();

	// backup original text
	QString oldtext = text();

	// button text larger than what will fit?
	if(w1 > w2) {
		if( !d->hasToolTip ) {
			QToolTip::add(this, text());
			d->hasToolTip = TRUE;
		}

		// make a string that fits
		bool found = FALSE;
		QString newtext;
		int n;
		for(n = oldtext.length(); n > 0; --n) {
			if(fm.width(oldtext, n) < w2) {
				found = TRUE;
				break;
			}
		}
		if(found)
			newtext = oldtext.mid(0, n);
		else
			newtext = "";

		// set the new text that fits.  updates must be off, or we recurse.
		setUpdatesEnabled(FALSE);
		QButton::setText(newtext);
		setUpdatesEnabled(TRUE);
	}
	else {
		if( d->hasToolTip ) {
			QToolTip::remove(this);
			d->hasToolTip = FALSE;
		}
	}

	// draw!
	QPushButton::drawButtonLabel(p);

	// restore original button text now that we are done drawing.
	setUpdatesEnabled(FALSE);
	QButton::setText(oldtext);
	setUpdatesEnabled(TRUE);

#else*/
	QPushButton::drawButtonLabel(p);
//#endif
}

#include "iconwidget.moc"
