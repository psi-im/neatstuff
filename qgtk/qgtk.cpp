#include <gtk/gtk.h>

#include "qgtk.h"

#include <qpixmap.h>

#include "fakegdkwindow.h"

class QGtk::Private
{
public:
	Private() {}

	GtkStyle *style;
	FakeGdkWindowSupplier *ws;
};

QGtk::QGtk()
: QMotifPlusStyle()
{
	d = new Private;
	gtk_init(0, 0);
	d->ws = new FakeGdkWindowSupplier(this);

	d->style = gtk_widget_get_default_style();
	g_object_ref(d->style);

	gtk_style_attach (d->style, (GdkWindow *)d->ws->getWindow());
}

QGtk::~QGtk()
{
	g_object_unref(d->style);
	delete d;
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
	d->ws->setPainter(paint);

	if ( ce == QStyle::CE_PushButton ) {
		/*gtk_paint_box (d->style, (GdkWindow *)d->ws->getWindow(),
				GTK_STATE_NORMAL, GTK_SHADOW_IN,
				0, 0, "buttondefault",
				rect.x(), rect.y(), rect.width(), rect.height());*/

		//widget = GTK_WIDGET (button);
		//border_width = 0; //GTK_CONTAINER (widget)->border_width;

		/*gtk_button_get_props (button, &default_border, &default_outside_border, &interior_focus);
		gtk_widget_style_get (GTK_WIDGET (widget),
					"focus-line-width", &focus_width,
					"focus-padding", &focus_pad,
					NULL);

		x = widget->allocation.x + border_width;
		y = widget->allocation.y + border_width;
		width = widget->allocation.width - border_width * 2;
		height = widget->allocation.height - border_width * 2;*/

		int x = rect.x(), y = rect.y(), width = rect.width(), height = rect.height();

		//if (GTK_WIDGET_HAS_DEFAULT (widget) &&
		//	GTK_BUTTON (widget)->relief == GTK_RELIEF_NORMAL)
		if ( flags & Style_ButtonDefault )
		{
			gtk_paint_box (d->style, (GdkWindow *)d->ws->getWindow(),
					GTK_STATE_NORMAL, GTK_SHADOW_IN,
					0, 0/*widget*/, "buttondefault",
					x, y, width, height);

			/*x += default_border.left;
			y += default_border.top;
			width -= default_border.left + default_border.right;
			height -= default_border.top + default_border.bottom;*/
		}
		/*else if (GTK_WIDGET_CAN_DEFAULT (widget))
		{
			x += default_outside_border.left;
			y += default_outside_border.top;
			width -= default_outside_border.left + default_outside_border.right;
			height -= default_outside_border.top + default_outside_border.bottom;
		}*/

		/*if (!interior_focus && GTK_WIDGET_HAS_FOCUS (widget))
		{
			x += focus_width + focus_pad;
			y += focus_width + focus_pad;
			width -= 2 * (focus_width + focus_pad);
			height -= 2 * (focus_width + focus_pad);
		}*/

		GtkStateType state = GTK_STATE_NORMAL;

		if ( flags & Style_HasFocus )
			state = GTK_STATE_ACTIVE;
		// baah

		/*if ((button->relief != GTK_RELIEF_NONE) ||
			((GTK_WIDGET_STATE(widget) != GTK_STATE_NORMAL) &&
			(GTK_WIDGET_STATE(widget) != GTK_STATE_INSENSITIVE)))
		{
			gtk_paint_box (d->style, (GdkWindow *)d->ws->getWindow(),
				state_type,
				shadow_type, area, widget, "button",
				x, y, width, height);
		}*/

		//if (GTK_WIDGET_HAS_FOCUS (widget))
		if ( flags & Style_HasFocus )
		{
			/*if (interior_focus)
			{
				x += widget->style->xthickness + focus_pad;
				y += widget->style->ythickness + focus_pad;
				width -= 2 * (widget->style->xthickness + focus_pad);
				height -=  2 * (widget->style->xthickness + focus_pad);
			}
			else
			{
				x -= focus_width + focus_pad;
				y -= focus_width + focus_pad;
				width += 2 * (focus_width + focus_pad);
				height += 2 * (focus_width + focus_pad);
			}*/

			gtk_paint_focus (d->style, (GdkWindow *)d->ws->getWindow(),
					state,
					0/*area*/, 0/*widget*/, "button",
					x, y, width, height);
		}
	}
	else if ( ce == QStyle::CE_PushButtonLabel ) {
	}
	else
		QMotifPlusStyle::drawControl(ce, paint, widget, rect, cg, flags, so);

	qWarning("***");
/*
  if (GTK_WIDGET_DRAWABLE (button))
    {
      widget = GTK_WIDGET (button);
      border_width = GTK_CONTAINER (widget)->border_width;

      gtk_button_get_props (button, &default_border, &default_outside_border, &interior_focus);
      gtk_widget_style_get (GTK_WIDGET (widget),
			    "focus-line-width", &focus_width,
			    "focus-padding", &focus_pad,
			    NULL);

      x = widget->allocation.x + border_width;
      y = widget->allocation.y + border_width;
      width = widget->allocation.width - border_width * 2;
      height = widget->allocation.height - border_width * 2;

      if (GTK_WIDGET_HAS_DEFAULT (widget) &&
	  GTK_BUTTON (widget)->relief == GTK_RELIEF_NORMAL)
	{
	  gtk_paint_box (widget->style, widget->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_IN,
			 area, widget, "buttondefault",
			 x, y, width, height);

	  x += default_border.left;
	  y += default_border.top;
	  width -= default_border.left + default_border.right;
	  height -= default_border.top + default_border.bottom;
	}
      else if (GTK_WIDGET_CAN_DEFAULT (widget))
	{
	  x += default_outside_border.left;
	  y += default_outside_border.top;
	  width -= default_outside_border.left + default_outside_border.right;
	  height -= default_outside_border.top + default_outside_border.bottom;
	}

      if (!interior_focus && GTK_WIDGET_HAS_FOCUS (widget))
	{
	  x += focus_width + focus_pad;
	  y += focus_width + focus_pad;
	  width -= 2 * (focus_width + focus_pad);
	  height -= 2 * (focus_width + focus_pad);
	}

      if ((button->relief != GTK_RELIEF_NONE) ||
	  ((GTK_WIDGET_STATE(widget) != GTK_STATE_NORMAL) &&
	   (GTK_WIDGET_STATE(widget) != GTK_STATE_INSENSITIVE)))
	gtk_paint_box (widget->style, widget->window,
		       state_type,
		       shadow_type, area, widget, "button",
		       x, y, width, height);

      if (GTK_WIDGET_HAS_FOCUS (widget))
	{
	  if (interior_focus)
	    {
	      x += widget->style->xthickness + focus_pad;
	      y += widget->style->ythickness + focus_pad;
	      width -= 2 * (widget->style->xthickness + focus_pad);
	      height -=  2 * (widget->style->xthickness + focus_pad);
	    }
	  else
	    {
	      x -= focus_width + focus_pad;
	      y -= focus_width + focus_pad;
	      width += 2 * (focus_width + focus_pad);
	      height += 2 * (focus_width + focus_pad);
	    }

	  gtk_paint_focus (widget->style, widget->window, GTK_WIDGET_STATE (widget),
			   area, widget, "button",
			   x, y, width, height);
	}
    }
*/
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
