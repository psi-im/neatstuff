#include "advwidget.h"

#include <qapplication.h>

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
	static int stickAt;

	void posChanging(int *x, int *y, int width, int height);
};

int AdvancedWidget::Private::stickAt = 5;

AdvancedWidget::Private::Private(AdvancedWidget *_parent)
{
	parent = _parent;
}

void AdvancedWidget::Private::posChanging(int *x, int *y, int width, int height)
{
	// maybe cache this value?
	QRect desktop = qApp->desktop()->availableGeometry(parent);

	if ( *x <= desktop.left() + stickAt &&
	     *x > -stickAt )
		*x = desktop.left();

	if ( *x + width > desktop.right() - stickAt &&
	     *x + width < desktop.right() + stickAt )
		*x = desktop.right() - width + 1;

	if ( *y <= desktop.top() + stickAt &&
	     *y > -stickAt )
		*y = desktop.top();

	if ( *y + height > desktop.bottom() - stickAt &&
	     *y + height < desktop.bottom() + stickAt )
		*y = desktop.bottom() - height + 1;
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
