#include "advwidget.h"

#include <qapplication.h>
#include <qwidgetlist.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#endif

//----------------------------------------------------------------------------
// AdvancedWidget::Private
//----------------------------------------------------------------------------

class AdvancedWidget::Private
{
public:
	Private(AdvancedWidget *parent);

	AdvancedWidget *parent;

	static int  stickAt;
	static bool stickToWindows;
	static bool stickEnabled;

	void posChanging(int *x, int *y, int width, int height);
};

int  AdvancedWidget::Private::stickAt = 5;
bool AdvancedWidget::Private::stickToWindows = true;
bool AdvancedWidget::Private::stickEnabled = true;

AdvancedWidget::Private::Private(AdvancedWidget *_parent)
{
	parent = _parent;
}

void AdvancedWidget::Private::posChanging(int *x, int *y, int width, int height)
{
	if ( stickAt <= 0 || !stickEnabled )
		return;

	QWidget *w;
	QDesktopWidget *desktop = qApp->desktop();

	QWidgetList *list;
	if ( stickToWindows )
		list = QApplication::topLevelWidgets();
	else
		list = new QWidgetList();
	list->append( desktop );
	QWidgetListIt it( *list );

	for ( ; (w = it.current()); ++it ) {
		QRect rect;
		if ( w->isDesktop() )
			rect = ((QDesktopWidget *)w)->availableGeometry(parent);
		else {
			if ( w == parent ||
			     desktop->screenNumber(parent) != desktop->screenNumber(w) )
				continue;

			// we want for widget to stick to outer edges of another widget, so
			// we'll change the rect to what it'll snap
			rect = QRect(w->frameGeometry().bottomRight(), w->frameGeometry().topLeft());
		}

		if ( *x <= rect.left() + stickAt &&
		     *x >  rect.left() - stickAt )
			*x = rect.left();

		if ( *x + width > rect.right() - stickAt &&
		     *x + width < rect.right() + stickAt )
			*x = rect.right() - width + 1;

		if ( *y <= rect.top() + stickAt &&
		     *y >  rect.top() - stickAt )
			*y = rect.top();

		if ( *y + height > rect.bottom() - stickAt &&
		     *y + height < rect.bottom() + stickAt )
			*y = rect.bottom() - height + 1;
	}

	delete list;
}

//----------------------------------------------------------------------------
// AdvancedWidget
//----------------------------------------------------------------------------

AdvancedWidget::AdvancedWidget(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	d = new Private(this);
}

AdvancedWidget::~AdvancedWidget()
{
	delete d;
}

#ifdef Q_OS_WIN
bool AdvancedWidget::winEvent(MSG *msg)
{
	if ( msg->message == WM_WINDOWPOSCHANGING ) {
		WINDOWPOS *wpos = (WINDOWPOS *)msg->lParam;

		d->posChanging(&wpos->x, &wpos->y, wpos->cx, wpos->cy);

		return true;
	}

	return false;
}
#endif

int AdvancedWidget::stickAt()
{
	return Private::stickAt;
}

void AdvancedWidget::setStickAt(int val)
{
	Private::stickAt = val;
}

bool AdvancedWidget::stickToWindows()
{
	return Private::stickToWindows;
}

void AdvancedWidget::setStickToWindows(bool val)
{
	Private::stickToWindows = val;
}

bool AdvancedWidget::stickEnabled()
{
	return Private::stickEnabled;
}

void AdvancedWidget::setStickEnabled(bool val)
{
	Private::stickEnabled = val;
}
