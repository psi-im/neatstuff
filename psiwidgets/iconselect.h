#ifndef ICONSELECT_H
#define ICONSELECT_H

#include <qpopupmenu.h>

class Icon;
class Iconset;

class IconSelectPopup : public QPopupMenu
{
	Q_OBJECT

public:
	IconSelectPopup(QWidget *parent = 0, const char *name = 0);
	~IconSelectPopup();

	void setIconset(const Iconset &);
	const Iconset &iconset() const;

signals:
	void iconSelected(const Icon *);

private:
	class Private;
	Private *d;
};

#endif
