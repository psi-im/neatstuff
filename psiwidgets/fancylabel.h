/****************************************************************************
** fancylabel.h - the FancyLabel widget
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

#ifndef FANCYLABEL_H
#define FANCYLABEL_H

#include <qwidget.h>
#include <qlabel.h>

class QPixmap;
class QColor;
class Icon;

class FancyLabel : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
	Q_PROPERTY( QString text READ text WRITE setText )
	Q_PROPERTY( QString help READ help WRITE setHelp )
	Q_PROPERTY( QColor colorFrom READ colorFrom WRITE setColorFrom )
	Q_PROPERTY( QColor colorTo READ colorTo WRITE setColorTo )
	Q_PROPERTY( QString iconName READ iconName WRITE setIcon )

	Q_ENUMS( Shape Shadow )
	Q_PROPERTY( Shape frameShape READ frameShape WRITE setFrameShape )
	Q_PROPERTY( Shadow frameShadow READ frameShadow WRITE setFrameShadow )
	Q_PROPERTY( int lineWidth READ lineWidth WRITE setLineWidth )
	Q_PROPERTY( int midLineWidth READ midLineWidth WRITE setMidLineWidth )

public:
	FancyLabel (QWidget *parent = 0, const char *name = 0);
	~FancyLabel ();
	
	const QString &text () const;
	const QString &help () const;
	const QPixmap *pixmap () const;
	const QColor &colorFrom () const;
	const QColor &colorTo () const;
	const Icon *icon () const;
	const QString &iconName () const;
	void setText (const QString &);
	void setHelp (const QString &);
	void setPixmap (const QPixmap &);
	void setColorFrom (const QColor &);
	void setColorTo (const QColor &);
	void setIcon (const Icon *);
	void setIcon (const QString &);

	enum Shape { NoFrame  = 0,                  // no frame
		     Box      = 0x0001,             // rectangular box
		     Panel    = 0x0002,             // rectangular panel
		     WinPanel = 0x0003,             // rectangular panel (Windows)
		     StyledPanel = 0x0006,          // rectangular panel depending on the GUI style
		     PopupPanel = 0x0007,           // rectangular panel depending on the GUI style
		     MenuBarPanel = 0x0008,
		     ToolBarPanel = 0x0009,
		     LineEditPanel = 0x000a,
		     TabWidgetPanel = 0x000b,
		     GroupBoxPanel = 0x000c,
		     MShape   = 0x000f              // mask for the shape
	};
	enum Shadow { Plain    = 0x0010,            // plain line
		      Raised   = 0x0020,            // raised shadow effect
		      Sunken   = 0x0030,            // sunken shadow effect
		      MShadow  = 0x00f0 };          // mask for the shadow

	Shape frameShape () const;
	void setFrameShape (Shape);
	
	Shadow frameShadow () const;
	void setFrameShadow (Shadow);

	int lineWidth () const;
	void setLineWidth (int);

	int midLineWidth () const;
	void setMidLineWidth (int);
	
private:
	class Private;
	Private *d;
};

class IconLabel : public QLabel
{
	Q_OBJECT
	Q_PROPERTY( QString iconName READ iconName WRITE setIcon )
	
public:
	IconLabel(QWidget *parent = 0, const char *name = 0);
	~IconLabel();
	
	const Icon *icon () const;
	const QString &iconName () const;
	void setIcon (const Icon *);
	void setIcon (const QString &);

	class Private;
private:
	void drawContents (QPainter *paint);

	Private *d;
};

#endif
