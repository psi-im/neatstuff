#ifndef QGTK_H
#define QGTK_H

#include <qmotifplusstyle.h>

class QGtk : public QMotifPlusStyle
{
	Q_OBJECT

public:
	QGtk();
	~QGtk();

	virtual void polishPopupMenu(QPopupMenu *);
	virtual void drawPrimitive(QStyle::PrimitiveElement, QPainter*, const QRect&, const QColorGroup&, unsigned int = Style_Default, const QStyleOption& = QStyleOption::Default) const;
	virtual void drawControl(QStyle::ControlElement, QPainter*, const QWidget*, const QRect&, const QColorGroup&, unsigned int = Style_Default, const QStyleOption& = QStyleOption::Default) const;
	virtual void drawControlMask(QStyle::ControlElement, QPainter*, const QWidget*, const QRect&, const QStyleOption& = QStyleOption::Default) const;
	virtual QRect subRect(QStyle::SubRect, const QWidget*) const;
	virtual void drawComplexControl(QStyle::ComplexControl, QPainter*, const QWidget*, const QRect&, const QColorGroup&, unsigned int = Style_Default, unsigned int = SC_All, unsigned int = SC_None, const QStyleOption& = QStyleOption::Default) const;
	virtual void drawComplexControlMask(QStyle::ComplexControl, QPainter*, const QWidget*, const QRect&, const QStyleOption& = QStyleOption::Default) const;
	virtual QRect querySubControlMetrics(QStyle::ComplexControl, const QWidget*, QStyle::SubControl, const QStyleOption& = QStyleOption::Default) const;
	virtual QStyle::SubControl querySubControl(QStyle::ComplexControl, const QWidget*, const QPoint&, const QStyleOption& = QStyleOption::Default) const;
	virtual int pixelMetric(QStyle::PixelMetric, const QWidget* = 0) const;
	virtual QSize sizeFromContents(QStyle::ContentsType, const QWidget*, const QSize&, const QStyleOption& = QStyleOption::Default) const;
	virtual int styleHint(QStyle::StyleHint, const QWidget* = 0, const QStyleOption& = QStyleOption::Default, QStyleHintReturn* = 0) const;
	virtual QPixmap stylePixmap(QStyle::StylePixmap, const QWidget* = 0, const QStyleOption& = QStyleOption::Default) const;

private:
	class Private;
	Private *d;
};

#endif
