#include "advwidget.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qtimer.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winuser.h>
#endif

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
// AdvancedWidget::Private
//----------------------------------------------------------------------------

class AdvancedWidget::Private : QObject
{
public:
	Private(AdvancedWidget *parent);

	static int  stickAt;
	static bool stickToWindows;
	static bool stickEnabled;

	void posChanging(int *x, int *y, int *width, int *height);
};

int  AdvancedWidget::Private::stickAt        = 5;
bool AdvancedWidget::Private::stickToWindows = true;
bool AdvancedWidget::Private::stickEnabled   = true;

AdvancedWidget::Private::Private(AdvancedWidget *parent)
	: QObject(parent)
{
	if ( !advancedWidgetShared )
		advancedWidgetShared = new AdvancedWidgetShared();
}

void AdvancedWidget::Private::posChanging(int *x, int *y, int *width, int *height)
{
	if ( stickAt <= 0 || !stickEnabled )
		return;

	QWidget *p = (QWidget *)parent();
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
			     (dockWidget && (p->frameGeometry().bottom() >= rect.bottom() &&
					     p->frameGeometry().top() <= rect.top())) ) {
				*x = rect.left();
				if ( resizing )
					*width = p->frameSize().width() + p->x() - *x;
			}
		}

		if ( *x + *width > rect.right() - stickAt &&
		     *x + *width < rect.right() + stickAt ) {
			if ( !dockWidget ||
			     (dockWidget && (p->frameGeometry().bottom() >= rect.bottom() &&
					     p->frameGeometry().top() <= rect.top())) ) {
				if ( resizing )
					*width = p->frameSize().width() + *x - p->x();
				*x = rect.right() - *width + 1;
			}
		}

		if ( *y != p->y() )
		if ( *y <= rect.top() + stickAt &&
		     *y >  rect.top() - stickAt ) {
			if ( !dockWidget ||
			     (dockWidget && (p->frameGeometry().right() >= rect.right() &&
					     p->frameGeometry().left() <= rect.left())) ) {
				*y = rect.top();
				if ( resizing )
					*height = p->frameSize().height() + p->y() - *y;
			}
		}

		if ( *y + *height > rect.bottom() - stickAt &&
		     *y + *height < rect.bottom() + stickAt ) {
			if ( !dockWidget ||
			     (dockWidget && (p->frameGeometry().right() >= rect.right() &&
					     p->frameGeometry().left() <= rect.left())) ) {
				if ( resizing )
					*height = p->frameSize().height() + *y - p->y();
				*y = rect.bottom() - *height + 1;
			}
		}
	}
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

		d->posChanging(&wpos->x, &wpos->y, &wpos->cx, &wpos->cy);

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

#include "advwidget.moc"
