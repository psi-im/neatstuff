#ifndef FANCYPOPUP_H
#define FANCYPOPUP_H

//#include "qpassivepopup.h"

#include <qframe.h>

class QTimer;

class FancyPopup : public QFrame //QPassivePopup
{
	Q_OBJECT
public:
	FancyPopup(FancyPopup *prev = 0, QString title = "", QString titleIcon = "", QString statusIconStr = "",
QString nameStr = "", QString JID = "", QString statusStr = "");
	~FancyPopup();

	void show();

	void prependPopup(const FancyPopup *);

signals:
	void clicked();

protected:
	void hideEvent(QHideEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void updateMask();
	void drawContents(QPainter *);

public:
	class Private;
private:
	Private *d;
	friend class Private;
};

#endif
