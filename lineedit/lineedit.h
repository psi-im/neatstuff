#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <qtextedit.h>

class LineEdit : public QTextEdit
{
	Q_OBJECT
public:
	LineEdit(QWidget *parent);
	~LineEdit();

	QSize sizeHint() const;

public slots:
	void recalculateSize();
};

#endif
