#include <gtk/gtk.h>

#include "fakegdkwindow.h"

#include <qpainter.h>

struct QGtkPainter
{
	static QPainter *painter;

	static GdkGC *create_gc(GdkDrawable *drawable, GdkGCValues *values, GdkGCValuesMask mask)
	{
		qWarning("QGtkPainter::create_gc -- not implemented yet");
		return 0;
	}

	static void draw_rectangle(GdkDrawable *drawable, GdkGC *gc, gboolean filled, gint x, gint y, gint width, gint height)
	{
		qWarning("QGtkPainter::draw_rectangle -- not implemented yet");

		GdkGCValues values;
		GDK_GC_GET_CLASS(gc)->get_values(gc, &values);

		GdkColor col = values.background; //foreground
		QColor foreground(col.red/256, col.green/256, col.blue/256);

		if ( filled )
			painter->setBrush( foreground );
		else
			painter->setPen( foreground );
		painter->drawRect(x, y, width, height);
	}

	static void draw_arc(GdkDrawable *drawable, GdkGC *gc, gboolean filled, gint x, gint y, gint width, gint height, gint angle1, gint angle2)
	{
		qWarning("QGtkPainter::draw_arc -- not implemented yet");
	}

	static void draw_polygon(GdkDrawable *drawable, GdkGC *gc, gboolean filled, GdkPoint *points, gint npoints)
	{
		qWarning("QGtkPainter::draw_polygon -- not implemented yet");
	}

	static void draw_text(GdkDrawable *drawable, GdkFont *font, GdkGC *gc, gint x, gint y, const gchar *text, gint text_length)
	{
		qWarning("QGtkPainter::draw_text -- not implemented yet");
	}

	static void draw_text_wc(GdkDrawable *drawable, GdkFont *font, GdkGC *gc, gint x, gint y, const GdkWChar *text, gint text_length)
	{
		qWarning("QGtkPainter::draw_text_wc -- not implemented yet");
	}

	static void draw_drawable(GdkDrawable *drawable, GdkGC *gc, GdkDrawable *src, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height)
	{
		qWarning("QGtkPainter::draw_drawable -- not implemented yet");
	}

	static void draw_points(GdkDrawable *drawable, GdkGC *gc, GdkPoint *points, gint npoints)
	{
		qWarning("QGtkPainter::draw_points -- not implemented yet");
	}

	static void draw_segments(GdkDrawable *drawable, GdkGC *gc, GdkSegment *segs, gint nsegs)
	{
		qWarning("QGtkPainter::draw_segments -- not implemented yet");
	}

	static void draw_lines(GdkDrawable *drawable, GdkGC *gc, GdkPoint *points, gint npoints)
	{
		qWarning("QGtkPainter::draw_lines -- not implemented yet");
	}

	static void draw_glyphs(GdkDrawable *drawable, GdkGC *gc, PangoFont *font, gint x, gint y, PangoGlyphString *glyphs)
	{
		qWarning("QGtkPainter::draw_glyphs -- not implemented yet");
	}

	static void draw_image(GdkDrawable *drawable, GdkGC *gc, GdkImage *image, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height)
	{
		qWarning("QGtkPainter::draw_image -- not implemented yet");
	}

	static gint get_depth(GdkDrawable *drawable)
	{
		qWarning("QGtkPainter::get_depth -- not implemented yet");
		return 24;
	}

	static void get_size(GdkDrawable *drawable, gint *width, gint *height)
	{
		qWarning("QGtkPainter::get_size -- not implemented yet");
	}

	static void set_colormap(GdkDrawable *drawable, GdkColormap *cmap)
	{
		qWarning("QGtkPainter::set_colormap -- not implemented yet");
	}

	static GdkColormap *get_colormap (GdkDrawable *drawable)
	{
		qWarning("QGtkPainter::get_colormap -- not implemented yet");
		return 0;
	}

	static GdkVisual *get_visual(GdkDrawable *drawable)
	{
		qWarning("QGtkPainter::get_visual -- not implemented yet");
		return 0;
	}

	static GdkScreen *get_screen(GdkDrawable *drawable)
	{
		qWarning("QGtkPainter::get_screen -- not implemented yet");
		return 0;
	}

	static GdkImage *get_image(GdkDrawable *drawable, gint x, gint y, gint width, gint height)
	{
		qWarning("QGtkPainter::get_image -- not implemented yet");
		return 0;
	}

	static GdkRegion *get_clip_region(GdkDrawable *drawable)
	{
		qWarning("QGtkPainter::get_clip_region -- not implemented yet");
		return 0;
	}

	static GdkRegion *get_visible_region(GdkDrawable *drawable)
	{
		qWarning("QGtkPainter::get_visible_region -- not implemented yet");
		return 0;
	}

	static GdkDrawable *get_composite_drawable(GdkDrawable *drawable, gint x, gint y, gint width, gint height, gint *composite_x_offset, gint *composite_y_offset)
	{
		qWarning("QGtkPainter::get_composite_drawable -- not implemented yet");
		return 0;
	}

	static void draw_pixbuf(GdkDrawable *drawable, GdkGC *gc, GdkPixbuf *pixbuf, gint src_x, gint src_y, gint dest_x, gint dest_y, gint width, gint height, GdkRgbDither dither, gint x_dither, gint y_dither)
	{
		qWarning("QGtkPainter::draw_pixbuf -- not implemented yet");
	}

	static GdkImage *_copy_to_image(GdkDrawable *drawable, GdkImage *image, gint src_x, gint src_y, gint dest_x, gint dest_y, gint width, gint height)
	{
		qWarning("QGtkPainter::_copy_to_image -- not implemented yet");
		return 0;
	}
};

QPainter *QGtkPainter::painter = 0;


class FakeGdkWindowSupplier::Private
{
public:
	FakeGdkWindow *window;

	Private()
	{
		gtk_init (0, 0);

		GdkWindowAttr attributes;
		attributes.window_type = /*GDK_WINDOW_TEMP;*/ GDK_WINDOW_TOPLEVEL;
		attributes.wclass = GDK_INPUT_OUTPUT;
		gint attributes_mask = 0;
		window = (FakeGdkWindow *)gdk_window_new(0, &attributes, attributes_mask);

		// setting our drawing functions
		GdkDrawableClass *drawable = GDK_DRAWABLE_GET_CLASS(window);

		drawable->create_gc			= QGtkPainter::create_gc;
		drawable->draw_rectangle		= QGtkPainter::draw_rectangle;
		drawable->draw_arc			= QGtkPainter::draw_arc;
		drawable->draw_polygon			= QGtkPainter::draw_polygon;
		drawable->draw_text			= QGtkPainter::draw_text;
		drawable->draw_text_wc			= QGtkPainter::draw_text_wc;
		drawable->draw_drawable			= QGtkPainter::draw_drawable;
		drawable->draw_points			= QGtkPainter::draw_points;
		drawable->draw_segments			= QGtkPainter::draw_segments;
		drawable->draw_lines			= QGtkPainter::draw_lines;
		drawable->draw_glyphs			= QGtkPainter::draw_glyphs;
		drawable->draw_image			= QGtkPainter::draw_image;
		drawable->get_depth			= QGtkPainter::get_depth;
		drawable->get_size			= QGtkPainter::get_size;
		//drawable->set_colormap			= QGtkPainter::set_colormap;
		//drawable->get_colormap			= QGtkPainter::get_colormap;
		//drawable->get_visual			= QGtkPainter::get_visual;
		//drawable->get_screen			= QGtkPainter::get_screen;
		drawable->get_image			= QGtkPainter::get_image;
		drawable->get_clip_region		= QGtkPainter::get_clip_region;
		drawable->get_visible_region		= QGtkPainter::get_visible_region;
		drawable->get_composite_drawable	= QGtkPainter::get_composite_drawable;
		drawable->draw_pixbuf			= QGtkPainter::draw_pixbuf;
		drawable->_copy_to_image		= QGtkPainter::_copy_to_image;
	}

	~Private()
	{
		g_object_unref( window );
	}
};

// supplier class

FakeGdkWindowSupplier::FakeGdkWindowSupplier(QObject *parent, const char *name)
: QObject(parent, name)
{
	d = new Private;
}

FakeGdkWindowSupplier::~FakeGdkWindowSupplier()
{
	delete d;
}

void FakeGdkWindowSupplier::setPainter(QPainter *painter)
{
	QGtkPainter::painter = painter;
}

FakeGdkWindow *FakeGdkWindowSupplier::getWindow()
{
	return d->window;
}

#if 0
// fake gdk window, that initializes drawing functions

struct FakeGdkWindow
{
	GdkWindowObject parent_instance;
};

struct FakeGdkWindowClass
{
	GdkWindowObjectClass parent_class;
};

static void fakeGdkWindow_init (FakeGdkWindow *)
{
}

static void fakeGdkWindowClass_init (FakeGdkWindowClass *klass, QGtkPainter *painter)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GdkDrawableClass *drawable_class = GDK_DRAWABLE_CLASS (klass);
	GdkWindowObjectClass *window_class = GDK_WINDOW_CLASS (klass);

	gpointer parent_class = g_type_class_peek_parent (klass);

	/*object_class->finalize = gdk_window_impl_x11_finalize;

	drawable_class->set_colormap = gdk_window_impl_x11_set_colormap;
	drawable_class->get_colormap = gdk_window_impl_x11_get_colormap;
	drawable_class->get_size = gdk_window_impl_x11_get_size;

	drawable_class->get_clip_region = gdk_window_impl_x11_get_visible_region;
	drawable_class->get_visible_region = gdk_window_impl_x11_get_visible_region;*/

	//drawable_class->blah = painter->blah;
}

static GType fakeGdkWindow_get_type (QGtkPainter *painter)
{
	static GType object_type = 0;

	if ( !object_type ) {
		static const GTypeInfo object_info = {
			sizeof (FakeGdkWindowClass),			// class size
			(GBaseInitFunc) 0,				// base init
			(GBaseFinalizeFunc) 0,				// base finalize

			(GClassInitFunc) fakeGdkWindowClass_init,	// class init
			0,						// class finalize
			painter, // FIXME				// class data

			sizeof (FakeGdkWindow),				// instance size
			0,						// n_preallocs
			(GInstanceInitFunc) fakeGdkWindow_init,		// instance init

			0						// value table
		};

		object_type = g_type_register_static (GDK_TYPE_WINDOW, "FakeGdkWindow", &object_info, (GTypeFlags)0);
	}

	return object_type;
}
#endif

