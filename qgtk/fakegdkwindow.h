#ifndef FAKEGDKWINDOW_H
#define FAKEGDKWINDOW_H

#include <qobject.h>

class QPainter;

struct FakeGdkWindow;

class FakeGdkWindowSupplier : public QObject
{
	Q_OBJECT

public:
	FakeGdkWindowSupplier(QObject *parent = 0, const char *name = 0);
	~FakeGdkWindowSupplier();

	FakeGdkWindow *getWindow();

	void setPainter(QPainter *);

private:
	class Private;
	Private *d;
};

#endif
