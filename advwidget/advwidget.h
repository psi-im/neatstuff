/*
 * advwidget.h - AdvancedWidget template class
 * Copyright (C) 2005  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef ADVWIDGET_H
#define ADVWIDGET_H

#include <qwidget.h>

class GAdvancedWidget : public QObject
{
	Q_OBJECT
public:
	GAdvancedWidget(QWidget *parent, const char *name = 0);
	~GAdvancedWidget();

	static bool stickEnabled();
	static void setStickEnabled(bool val);

	static int stickAt();
	static void setStickAt(int val);

	static bool stickToWindows();
	static void setStickToWindows(bool val);

	void doFlash(bool on);

#ifdef Q_OS_WIN
	bool winEvent(MSG *msg);
#endif

	void preSetCaption();
	void postSetCaption();

	void windowActivationChange(bool oldstate);

public:
	class Private;
private:
	Private *d;
};

template <class BaseClass>
class AdvancedWidget : public BaseClass
{
private:
	GAdvancedWidget *gAdvWidget;

public:
	AdvancedWidget( QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0 )
		: BaseClass( parent, name, f )
	{
		gAdvWidget = new GAdvancedWidget( this );
	}

	static bool stickEnabled() { return GAdvancedWidget::stickEnabled(); }
	static void setStickEnabled( bool val ) { GAdvancedWidget::setStickEnabled( val ); }

	static int stickAt() { return GAdvancedWidget::stickAt(); }
	static void setStickAt( int val ) { GAdvancedWidget::setStickAt( val ); }

	static bool stickToWindows() { return GAdvancedWidget::stickToWindows(); }
	static void setStickToWindows( bool val ) { GAdvancedWidget::setStickToWindows( val ); }

	void doFlash( bool on )
	{
		gAdvWidget->doFlash( on );
	}

#ifdef Q_OS_WIN
	bool winEvent( MSG *msg )
	{
		return gAdvWidget->winEvent( msg );
	}
#endif

	void setCaption( const QString &c )
	{
		gAdvWidget->preSetCaption();
		BaseClass::setCaption( c );
		gAdvWidget->postSetCaption();
	}

protected:
	void windowActivationChange( bool oldstate )
	{
		gAdvWidget->windowActivationChange( oldstate );
		BaseClass::windowActivationChange( oldstate );
	}
};

#endif
