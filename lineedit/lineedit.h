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
	void keyPressEvent( QKeyEvent *e );

signals:
	void addText(const QString &);

public slots:
	void recalculateSize();

private:
	class Private;
	Private *d;
};

#endif
