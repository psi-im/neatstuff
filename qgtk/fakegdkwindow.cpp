#include <gtk/gtk.h>

#include "fakegdkwindow.h"

#include <qpainter.h>

struct QGtkPainter
{
	QPainter *painter;
};

// fake gdk window, that initializes drawing functions

struct FakeGdkWindow
{
	GdkWindow parent_instance;
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
			sizeof (FakeGdkWindowClass),
			(GBaseInitFunc) 0,
			(GBaseFinalizeFunc) 0,
			(GClassInitFunc) fakeGdkWindowClass_init,
			0,
			painter, // FIXME
			sizeof (FakeGdkWindow),
			0,
			(GInstanceInitFunc) fakeGdkWindow_init,
			0
		};

		object_type = g_type_register_static (GDK_TYPE_DRAWABLE, "FakeGdkWindow", &object_info, (GTypeFlags) 0);
	}

	return object_type;
}

// class with actual drawing functions

// FIXME: QGtkPainter should be made global static

class FakeGdkWindowSupplier::Private
{
public:
	FakeGdkWindow *window;
	QGtkPainter *qgtkPainter;

	Private()
	{
		qgtkPainter = new QGtkPainter; // FIXME
		window = (FakeGdkWindow *)g_object_new( fakeGdkWindow_get_type(qgtkPainter), NULL );
	}

	~Private()
	{
		g_object_unref( window );
		delete qgtkPainter; // FIXME
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
	d->qgtkPainter->painter = painter;
}

FakeGdkWindow *FakeGdkWindowSupplier::getWindow()
{
	return d->window;
}
