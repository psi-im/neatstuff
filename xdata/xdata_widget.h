#ifndef XDATAWIDGET_H
#define XDATAWIDGET_H

#include <qwidget.h>

#include "xmpp_xdata.h"

using namespace Jabber;

class XDataWidget : public QWidget
{
public:
	XDataWidget(QWidget *parent = 0, const char *name = 0);
	~XDataWidget();

	XData::FieldList fields() const;
	void setFields(const XData::FieldList &);

private:
	class Private;
	Private *d;
};

#endif
