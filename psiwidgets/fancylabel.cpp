/****************************************************************************
** fancylabel.cpp - the FancyLabel widget
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

#include "fancylabel.h"

#include <qpixmap.h>
#include <qcolor.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpainter.h>
#include <qobjectlist.h>

#ifndef WIDGET_PLUGIN
#include "iconset.h"
#endif

//----------------------------------------------------------------------------
// IconLabel
//----------------------------------------------------------------------------

class IconLabel::Private : public QObject
{
	Q_OBJECT

public:

	IconLabel *label;
	Icon *icon;
#ifdef WIDGET_PLUGIN
	QString iconName;
#endif
	
	Private(IconLabel *l) 
	{
		label = l;
		icon = 0;
	}
	
	~Private() 
	{
		stopIcon();
	}

	void setIcon(const Icon *i)
	{
		stopIcon();
		icon = (Icon *)i;
		startIcon();
	}
	
protected:
	void stopIcon()
	{
#ifndef WIDGET_PLUGIN
		if ( icon ) {
			disconnect(icon, 0, this, 0 );
			icon->stop();
		}
#endif
	}

	void startIcon()
	{
#ifndef WIDGET_PLUGIN
		if ( icon ) {
			connect(icon, SIGNAL(pixmapChanged(const QPixmap &)), SLOT(iconUpdated(const QPixmap &)));
			icon->activated(false); // TODO: should icon play sound when it's activated on icon?
			iconUpdated( icon->pixmap() );
		}
		else
			iconUpdated( QPixmap() );
#endif
	}

private slots:
	void iconUpdated(const QPixmap &pix)
	{
		label->setPixmap(pix);
	}
};

IconLabel::IconLabel(QWidget *parent, const char *name)
: QLabel(parent, name, WRepaintNoErase | WResizeNoErase)
{
	d = new Private(this);
}

IconLabel::~IconLabel()
{
	delete d;
}

void IconLabel::drawContents (QPainter *paint)
{
	QWidget *pw = (QWidget *)parent();
	if ( !pw || !pw->paletteBackgroundPixmap() )
		return QLabel::drawContents(paint);

	QRect r = contentsRect();
	QPixmap pix(r.width(), r.height());
	QPainter p;
	p.begin(&pix);
	p.drawTiledPixmap (r, *pw->paletteBackgroundPixmap(), mapToParent( r.topLeft() ));
	QLabel::drawContents ( &p );
	p.end();

	paint->drawPixmap (contentsRect().topLeft(), pix );
}

const Icon *IconLabel::icon () const
{
	return d->icon;
}

const QString &IconLabel::iconName () const
{
#ifndef WIDGET_PLUGIN
	if ( d->icon )
		return d->icon->name();
	return QString::null;
#else
	return d->iconName;
#endif
}

void IconLabel::setIcon (const Icon *i)
{
	d->setIcon(i);
}

void IconLabel::setIcon (const QString &name)
{
#ifndef WIDGET_PLUGIN
	setIcon( IconsetFactory::iconPtr(name) );
#else
	d->iconName = name;
#endif
}

//----------------------------------------------------------------------------
// MyFancyFrame -- internal
//----------------------------------------------------------------------------

class MyFancyFrame : public QFrame 
{
protected:
	void drawContents (QPainter *paint)
	{
		paint->drawPixmap (contentsRect().topLeft(), background);
	}
	
	void resizeEvent (QResizeEvent *e)
	{
		QFrame::resizeEvent (e);
		
		QRect rect = contentsRect();
		int w = rect.width();
		
		if ( rect.height() <= 0 || w <= 0 )
			return; // avoid crash
		
		int r1, g1, b1;
		from->rgb (&r1, &g1, &b1);
		int r2, g2, b2;
		to->rgb (&r2, &g2, &b2);

		float stepR = (float)(r2 - r1) / w;
		float stepG = (float)(g2 - g1) / w;
		float stepB = (float)(b2 - b1) / w;

		QPixmap pix (rect.width(), rect.height());
		QPainter p;
		p.begin (&pix);
		for (int i = 0; i < w; i++) {
			int r = (int)((float)r1 + stepR*i);
			int g = (int)((float)g1 + stepG*i);
			int b = (int)((float)b1 + stepB*i);

			p.setPen ( QColor( r, g, b ) );
			p.drawLine ( i, 0, i, rect.height() );
		}
		p.end ();
		
		QObjectList *l = queryList( "QLabel" );
		QObjectListIt it( *l );
		while ( it.current() ) {
			QLabel *child = (QLabel *)it.current();
			child->update();
			
			++it;
		}
		delete l;
		
		setUpdatesEnabled(false);
		setPaletteBackgroundPixmap(pix);
		setUpdatesEnabled(true);
		
		background = pix;
		update ();
	}
	
private:
	QColor *from, *to;
	QPixmap background;
	
public:
	MyFancyFrame (QWidget *parent, QColor *_from, QColor *_to, const char *name = 0, WFlags f = 0)
	: QFrame (parent, name, f)
	{
		from = _from;
		to = _to;
	}
	
	void repaintBackground()
	{
		QResizeEvent e( size(), size() );
		resizeEvent( &e );
	}
};

//----------------------------------------------------------------------------
// FancyLabel
//----------------------------------------------------------------------------

class FancyLabel::Private : public QObject
{
public:
	MyFancyFrame *frame;
	IconLabel *text, *help, *pix;
	QColor from, to;
	QString textStr, helpStr;
#ifdef WIDGET_PLUGIN
	QString iconName;
#endif
	
	Private (FancyLabel *parent)
	: QObject(parent), from(72, 172, 243), to(255, 255, 255)
	{
		QHBoxLayout *mainbox = new QHBoxLayout( parent, 0, 0 ); 

		frame = new MyFancyFrame ( parent, &from, &to, "fancy_frame" );
		frame->setFrameShape( QFrame::StyledPanel );
		frame->setFrameShadow( QFrame::Raised );
		
		QHBoxLayout *frameLayout = new QHBoxLayout( frame, 3, 0 ); 
		QVBoxLayout *layout = new QVBoxLayout( 0, 0, 0 ); 
		frameLayout->addLayout( layout );
		
		text = new IconLabel( frame, "help_label" );
		text->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, text->sizePolicy().hasHeightForWidth() ) );
		layout->addWidget( text );

		help = new IconLabel( frame, "help_label" );
		layout->addWidget( help );

		pix = new IconLabel( frame, "pixmap_label" );
		pix->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)5, 0, 0, help->sizePolicy().hasHeightForWidth() ) );
		frameLayout->addWidget( pix );
		
		mainbox->addWidget( frame );
	}
};

FancyLabel::FancyLabel (QWidget *parent, const char *name)
: QWidget (parent, name)
{
	d = new Private (this);
}

FancyLabel::~FancyLabel ()
{
}

void FancyLabel::setText (const QString &text)
{
	d->textStr = text;
	d->text->setText ("<b>" + text + "</b>");
}

void FancyLabel::setHelp (const QString &help)
{
	d->helpStr = help;
	d->help->setText ("<small>" + help + "</small>");
}

void FancyLabel::setPixmap (const QPixmap &pix)
{
	d->pix->setPixmap( pix );
}

void FancyLabel::setColorFrom (const QColor &col)
{
	d->from = col;
	d->frame->repaintBackground();
}

void FancyLabel::setColorTo (const QColor &col)
{
	d->to = col;
	d->frame->repaintBackground();
}

const QString &FancyLabel::text () const 
{
	return d->textStr;
}

const QString &FancyLabel::help () const 
{
	return d->helpStr;
}

const QPixmap *FancyLabel::pixmap () const 
{
	return d->pix->pixmap();
}

const QColor &FancyLabel::colorFrom () const
{
	return d->from;
}

const QColor &FancyLabel::colorTo () const
{
	return d->to;
}

const Icon *FancyLabel::icon () const
{
	return d->pix->icon();
}

void FancyLabel::setIcon (const Icon *i)
{
	d->pix->setIcon (i);
}

void FancyLabel::setIcon (const QString &name)
{
#ifndef WIDGET_PLUGIN
	setIcon( IconsetFactory::iconPtr(name) );
#else
	d->iconName = name;
#endif
}

const QString &FancyLabel::iconName () const
{
#ifndef WIDGET_PLUGIN
	return d->pix->iconName();
#else
	return d->iconName;
#endif
}

FancyLabel::Shape FancyLabel::frameShape () const
{
	return (FancyLabel::Shape)(int)d->frame->frameShape();
}

void FancyLabel::setFrameShape (FancyLabel::Shape v)
{
	d->frame->setFrameShape( (QFrame::Shape)(int)v );
	d->frame->repaintBackground();
}

FancyLabel::Shadow FancyLabel::frameShadow () const
{
	return (FancyLabel::Shadow)(int)d->frame->frameShadow();
}

void FancyLabel::setFrameShadow (FancyLabel::Shadow v)
{
	d->frame->setFrameShadow( (QFrame::Shadow)(int)v );
	d->frame->repaintBackground();
}

int FancyLabel::lineWidth () const
{
	return d->frame->lineWidth();
}

void FancyLabel::setLineWidth (int v)
{
	d->frame->setLineWidth(v);
	d->frame->repaintBackground();
}

int FancyLabel::midLineWidth () const
{
	return d->frame->midLineWidth();
}

void FancyLabel::setMidLineWidth (int v)
{
	d->frame->setMidLineWidth(v);
	d->frame->repaintBackground();
}

#include "fancylabel.moc"
