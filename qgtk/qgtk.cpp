#include "qgtk.h"

#include <qpixmap.h>

QGtk::QGtk()
: QMotifPlusStyle()
{
}

QGtk::~QGtk()
{
}

void QGtk::polishPopupMenu(QPopupMenu *pm)
{
	QMotifPlusStyle::polishPopupMenu(pm);
}

void QGtk::drawPrimitive(QStyle::PrimitiveElement pe, QPainter *paint, const QRect &rect, const QColorGroup &cg, unsigned int flags, const QStyleOption &so) const
{
	QMotifPlusStyle::drawPrimitive(pe, paint, rect, cg, flags, so);
}

void QGtk::drawControl(QStyle::ControlElement ce, QPainter *paint, const QWidget *widget, const QRect &rect, const QColorGroup &cg, unsigned int flags, const QStyleOption &so) const
{
	QMotifPlusStyle::drawControl(ce, paint, widget, rect, cg, flags, so);
}

void QGtk::drawControlMask(QStyle::ControlElement ce, QPainter *paint, const QWidget *widget, const QRect &rect, const QStyleOption &so) const
{
	QMotifPlusStyle::drawControlMask(ce, paint, widget, rect, so);
}

QRect QGtk::subRect(QStyle::SubRect sr, const QWidget *widget) const
{
	return QMotifPlusStyle::subRect(sr, widget);
}

void QGtk::drawComplexControl(QStyle::ComplexControl cc, QPainter *paint, const QWidget *widget, const QRect &rect, const QColorGroup &cg, unsigned int how, unsigned int sub, unsigned int subActive, const QStyleOption &so) const
{
	QMotifPlusStyle::drawComplexControl(cc, paint, widget, rect, cg, how, sub, subActive, so);
}

void QGtk::drawComplexControlMask(QStyle::ComplexControl cc, QPainter *paint, const QWidget *widget, const QRect &rect, const QStyleOption &so) const
{
	QMotifPlusStyle::drawComplexControlMask(cc, paint, widget, rect, so);
}

QRect QGtk::querySubControlMetrics(QStyle::ComplexControl cc, const QWidget *widget, QStyle::SubControl sc, const QStyleOption &so) const
{
	return QMotifPlusStyle::querySubControlMetrics(cc, widget, sc, so);
}

QStyle::SubControl QGtk::querySubControl(QStyle::ComplexControl cc, const QWidget *widget, const QPoint &p, const QStyleOption &so) const
{
	return QMotifPlusStyle::querySubControl(cc, widget, p, so);
}

int QGtk::pixelMetric(QStyle::PixelMetric pm, const QWidget *widget) const
{
	return QMotifPlusStyle::pixelMetric(pm, widget);
}

QSize QGtk::sizeFromContents(QStyle::ContentsType ct, const QWidget *widget, const QSize &size, const QStyleOption &so) const
{
	return QMotifPlusStyle::sizeFromContents(ct, widget, size, so);
}

int QGtk::styleHint(QStyle::StyleHint sh, const QWidget *widget, const QStyleOption &so, QStyleHintReturn *shr) const
{
	return QMotifPlusStyle::styleHint(sh, widget, so, shr);
}

QPixmap QGtk::stylePixmap(QStyle::StylePixmap sp, const QWidget *widget, const QStyleOption &so) const
{
	return QMotifPlusStyle::stylePixmap(sp, widget, so);
}
