#ifndef POPUPLIST_H
#define POPUPLIST_H

#include <qobject.h>
#include <qptrlist.h>
#include "fancypopup.h"

// copied from psi/src/tasklist.h
class PopupList : public QObject, public QPtrList<FancyPopup>
{
	Q_OBJECT

public:
	PopupList()
	{
		setAutoDelete(true);
	}

	void prepend(const FancyPopup *d)
	{
		if ( isEmpty() )
			emit started();
		else {
			QPtrListIterator<FancyPopup> it(*this);
			FancyPopup *pop;
			while ( (pop = it.current()) ) {
				pop->prependPopup(d);

				++it;
			}
		}

		connect(d, SIGNAL(destroyed(QObject *)), SLOT(popupDestroyed(QObject *)));
		QPtrList<FancyPopup>::prepend(d);
	}

signals:
	void started();
	void finished();

private slots:
	void popupDestroyed(QObject *p)
	{
		setAutoDelete(false);
		remove((FancyPopup *)p);
		setAutoDelete(true);

		if ( isEmpty() )
			emit finished();
	}
};

#endif

