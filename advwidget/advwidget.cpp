/*
 * advwidget.cpp - AdvancedWidget template class
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

#include "advwidget.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qtimer.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#endif

// TODO: Make use of KDE taskbar flashing support
// TODO: Use FlashWindowEx instead of FlashWindow!

//----------------------------------------------------------------------------
// AdvancedWidgetShared
//----------------------------------------------------------------------------

class AdvancedWidgetShared : public QObject
{
	Q_OBJECT
public:
	AdvancedWidgetShared();
	~AdvancedWidgetShared();

	QWidgetList *topWidgets;

private:
	QTimer *topWidgetsTimer;

public slots:
	void startTimer();

private slots:
	void deleteTopWidgets();
};

AdvancedWidgetShared::AdvancedWidgetShared()
	: QObject(qApp)
{
	topWidgets = 0;
	topWidgetsTimer = new QTimer(this);
	connect(topWidgetsTimer, SIGNAL(timeout()), SLOT(deleteTopWidgets()));
}

AdvancedWidgetShared::~AdvancedWidgetShared()
{
	deleteTopWidgets();
}

void AdvancedWidgetShared::startTimer()
{
	topWidgetsTimer->start(250, true);
}

void AdvancedWidgetShared::deleteTopWidgets()
{
	if ( topWidgets ) {
		delete topWidgets;
		topWidgets = 0;
	}
}

static AdvancedWidgetShared *advancedWidgetShared = 0;

//----------------------------------------------------------------------------
// GAdvancedWidget::Private
//----------------------------------------------------------------------------

class GAdvancedWidget::Private : public QObject
{
	Q_OBJECT
public:
	Private(QWidget *parent);

	static int  stickAt;
	static bool stickToWindows;
	static bool stickEnabled;

	QWidget *parentWidget;

	bool flashing();
	void doFlash(bool on);
	void posChanging(int *x, int *y, int *width, int *height);

private:
	QTimer *flashTimer;
	int flashCount;

private slots:
	void flashAnimate();
};

int  GAdvancedWidget::Private::stickAt        = 5;
bool GAdvancedWidget::Private::stickToWindows = true;
bool GAdvancedWidget::Private::stickEnabled   = true;

GAdvancedWidget::Private::Private(QWidget *parent)
	: QObject(parent)
{
	if ( !advancedWidgetShared )
		advancedWidgetShared = new AdvancedWidgetShared();

	parentWidget = parent;
	flashTimer = 0;
	flashCount = 0;
}

void GAdvancedWidget::Private::posChanging(int *x, int *y, int *width, int *height)
{
	if ( stickAt <= 0 || !stickEnabled )
		return;

	QWidget *p = parentWidget;
	if ( p->pos() == QPoint(*x, *y) &&
	     p->frameSize() == QSize(*width, *height) )
		return;

	bool resizing = p->frameSize() != QSize(*width, *height);

	advancedWidgetShared->startTimer();

	QWidget *w;
	QDesktopWidget *desktop = qApp->desktop();
	QWidgetList *list = advancedWidgetShared->topWidgets;

	if ( !list ) {
		if ( stickToWindows )
			list = QApplication::topLevelWidgets();
		else
			list = new QWidgetList();
		list->append( desktop );

		advancedWidgetShared->topWidgets = list;
	}

	QWidgetListIt it( *list );
	for ( ; (w = it.current()); ++it ) {
		QRect rect;
		bool dockWidget = false;

		if ( w->isDesktop() )
			rect = ((QDesktopWidget *)w)->availableGeometry((QWidget *)parent());
		else {
			if ( w == p ||
			     desktop->screenNumber(p) != desktop->screenNumber(w) )
				continue;

			// we want for widget to stick to outer edges of another widget, so
			// we'll change the rect to what it'll snap
			rect = QRect(w->frameGeometry().bottomRight(), w->frameGeometry().topLeft());
			dockWidget = true;
		}

		if ( *x != p->x() )
		if ( *x <= rect.left() + stickAt &&
		     *x >  rect.left() - stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().bottom() >= rect.bottom() &&
			      p->frameGeometry().top() <= rect.top()) ) {
				*x = rect.left();
				if ( resizing )
					*width = p->frameSize().width() + p->x() - *x;
			}
		}

		if ( *x + *width >= rect.right() - stickAt &&
		     *x + *width <= rect.right() + stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().bottom() >= rect.bottom() &&
			      p->frameGeometry().top() <= rect.top()) ) {
				if ( resizing )
					*width = rect.right() - *x + 1;
				else
					*x = rect.right() - *width + 1;
			}
		}

		if ( *y != p->y() )
		if ( *y <= rect.top() + stickAt &&
		     *y >  rect.top() - stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().right() >= rect.right() &&
			      p->frameGeometry().left() <= rect.left()) ) {
				*y = rect.top();
				if ( resizing )
					*height = p->frameSize().height() + p->y() - *y;
			}
		}

		if ( *y + *height >= rect.bottom() - stickAt &&
		     *y + *height <= rect.bottom() + stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().right() >= rect.right() &&
			      p->frameGeometry().left() <= rect.left()) ) {
				if ( resizing )
					*height = rect.bottom() - *y + 1;
				else
					*y = rect.bottom() - *height + 1;
			}
		}
	}
}

void GAdvancedWidget::Private::doFlash(bool yes)
{
#ifdef Q_WS_WIN
	if ( yes ) {
		if ( flashTimer )
			return;
		flashTimer = new QTimer(this);
		connect(flashTimer, SIGNAL(timeout()), SLOT(flashAnimate()));
		flashCount = 0;
		flashAnimate(); // kick the first one immediately
		flashTimer->start(500);
	}
	else {
		if ( flashTimer ) {
			delete flashTimer;
			flashTimer = 0;
			// comment this out to fix titlebar repaint on Windows??
			//FlashWindow(winId(), false);
		}
	}
#else
	Q_UNUSED(yes)
#endif
}

void GAdvancedWidget::Private::flashAnimate()
{
#ifdef Q_WS_WIN
	FlashWindow( parentWidget->winId(), true );
	if ( ++flashCount == 5 )
		flashTimer->stop();
#endif
}

bool GAdvancedWidget::Private::flashing()
{
	bool on = false;
	if ( flashTimer )
		on = flashCount & 1;

	return on;
}

//----------------------------------------------------------------------------
// GAdvancedWidget
//----------------------------------------------------------------------------

GAdvancedWidget::GAdvancedWidget(QWidget *parent, const char *name)
	: QObject(parent, name)
{
	d = new Private(parent);
}

GAdvancedWidget::~GAdvancedWidget()
{
	delete d;
}

#ifdef Q_OS_WIN
bool GAdvancedWidget::winEvent(MSG *msg)
{
	if ( msg->message == WM_WINDOWPOSCHANGING ) {
		WINDOWPOS *wpos = (WINDOWPOS *)msg->lParam;

		d->posChanging(&wpos->x, &wpos->y, &wpos->cx, &wpos->cy);

		return true;
	}

	return false;
}
#endif

void GAdvancedWidget::preSetCaption()
{
#ifdef Q_WS_WIN
	if ( d->flashing() )
		FlashWindow( d->parentWidget->winId(), true );
#endif
}

void GAdvancedWidget::postSetCaption()
{
#ifdef Q_WS_WIN
	if ( d->flashing() )
		FlashWindow( d->parentWidget->winId(), true );
#endif
}

void GAdvancedWidget::doFlash(bool on)
{
	d->doFlash( d->parentWidget->isActiveWindow() && on );
}

void GAdvancedWidget::windowActivationChange(bool oldstate)
{
	if ( d->parentWidget->isActiveWindow() ) {
		d->doFlash(false);
	}
}

int GAdvancedWidget::stickAt()
{
	return Private::stickAt;
}

void GAdvancedWidget::setStickAt(int val)
{
	Private::stickAt = val;
}

bool GAdvancedWidget::stickToWindows()
{
	return Private::stickToWindows;
}

void GAdvancedWidget::setStickToWindows(bool val)
{
	Private::stickToWindows = val;
}

bool GAdvancedWidget::stickEnabled()
{
	return Private::stickEnabled;
}

void GAdvancedWidget::setStickEnabled(bool val)
{
	Private::stickEnabled = val;
}

#include "advwidget.moc"
