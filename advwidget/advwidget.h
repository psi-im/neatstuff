#ifndef ADVWIDGET_H
#define ADVWIDGET_H

#include <qwidget.h>

class AdvancedWidget : public QWidget
{
	Q_OBJECT
public:
	AdvancedWidget(QWidget *parent = 0, const char *name = 0);
	~AdvancedWidget();

	static int stickAt();
	static void setStickAt(int val);

protected:
#ifdef Q_OS_WIN
	bool winEvent(MSG *msg);
#endif

private:
	class Private;
	Private *d;
};

#endif
