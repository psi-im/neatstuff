#ifndef ADVWIDGET_H
#define ADVWIDGET_H

#include <qwidget.h>

class AdvancedWidget : public QWidget
{
	Q_OBJECT
public:
	AdvancedWidget(QWidget *parent = 0, const char *name = 0);
	~AdvancedWidget();

	static bool stickEnabled();
	static bool setStickEnabled(bool val);

	static int stickAt();
	static void setStickAt(int val);

	static bool stickToWindows();
	static void setStickToWindows(bool val);

protected:
#ifdef Q_OS_WIN
	bool winEvent(MSG *msg);
#endif

private:
	class Private;
	Private *d;
};

#endif
