#include "psitextview.h"

#include <qpopupmenu.h>

#include <qstylesheet.h>
#include <private/qrichtext_p.h>

#ifndef WIDGET_PLUGIN
#include "iconset.h"
#else
class Icon;
class Iconset;
#endif

//----------------------------------------------------------------------------
// URLObject - helper class for handling links
//----------------------------------------------------------------------------

class URLObject : public QObject
{
	Q_OBJECT

public:
	URLObject(QObject *parent = 0)
	: QObject(parent) {}

	QString link;

	QPopupMenu *createPopupMenu(const QString &l);

public slots:
	void popupAction() { }
	void popupCopy() { }
};

QPopupMenu *URLObject::createPopupMenu(const QString &l)
{
	link = l;
	if ( link.isEmpty() )
		return 0;

	int colon = link.find(':');
	if ( colon == -1 )
		colon = 0;
	QString service = link.left( colon );

	QString action = "ERROR";
	QString iconName;

	if ( service == "http" || service == "https" || service.isEmpty() ) {
		action = URLLabel::tr("Open web browser");
		iconName = "psi/www";
	}
	else if ( service == "mailto" ) {
		action = URLLabel::tr("Open mail composer");
		iconName = "psi/mail";
	}
	else if ( service == "jabber" || service == "jid" ) {
		// TODO: need more actions to jabber item. Ex: "add to roster", "send message"
		action = URLLabel::tr("Add to Roster");
		iconName = "psi/add";
	}

#ifndef WIDGET_PLUGIN
	const Icon *icon = 0;
	if ( !iconName.isEmpty() )
		icon = IconsetFactory::iconPtr( iconName );
#endif

	QPopupMenu *m = new QPopupMenu;

#ifndef WIDGET_PLUGIN
	if ( icon )
		m->insertItem(/*QIconSet(*/ *icon /*)*/, action, this, SLOT(popupAction()));
	else
#endif
		m->insertItem(action, this, SLOT(popupAction()));
	m->insertItem(URLLabel::tr("Copy location"), this, SLOT(popupCopy()));

	return m;
}

//----------------------------------------------------------------------------
// TextIcon - helper class for PsiTextView
//----------------------------------------------------------------------------

class TextIcon : public QObject, public QTextCustomItem
{
	Q_OBJECT

public:
	TextIcon(QTextDocument *p, const QMap<QString, QString> &attr, const QString &context, QMimeSourceFactory &factory);
	~TextIcon();

	void adjustToPainter(QPainter *);
	void draw(QPainter *p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup &cg, bool selected);
	QString richText() const;

	Placement placement() const { return place; }
	int minimumWidth() const { return width; }

private slots:
	void iconUpdated(const QPixmap &);

private:
	Placement place;
	int tmpwidth, tmpheight;
	QMap<QString, QString> attributes;
	QString imgId;
	Icon *icon;
};

TextIcon::TextIcon(QTextDocument *p, const QMap<QString, QString> &attr, const QString &context, QMimeSourceFactory &factory)
: QObject(0, 0), QTextCustomItem (p)
{
	width = height = 0;

	icon = 0;
	QString iconName = attr["name"];

	if ( iconName.isEmpty() )
		iconName = attr["src"];

	if ( iconName.isEmpty() )
		iconName = attr["source"];

#ifndef WIDGET_PLUGIN
	if ( !iconName.isEmpty() ) {
		icon = (Icon *)IconsetFactory::iconPtr( iconName );

		if ( icon ) {
			icon->activated();
			connect(icon, SIGNAL(pixmapChanged(const QPixmap &)), SLOT(iconUpdated(const QPixmap &)));

			width = icon->pixmap().width();
			height = icon->pixmap().height();
		}
	}
#endif

	if ( !icon && (width*height)==0 )
		width = height = 50;

	place = PlaceInline;
	if ( attr["align"] == "left" )
		place = PlaceLeft;
	else if ( attr["align"] == "right" )
		place = PlaceRight;

	tmpwidth = width;
	tmpheight = height;

	attributes = attr;
}

TextIcon::~TextIcon()
{
#ifndef WIDGET_PLUGIN
	if ( icon )
		icon->stop();
#endif
}

void TextIcon::adjustToPainter(QPainter *)
{
	// FIXME: This class can be incorrectly printed. Check this sometime later.
}

void TextIcon::draw(QPainter *p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup &cg, bool selected)
{
	if ( placement() != PlaceInline ) {
		x = xpos;
		y = ypos;
	}

	if ( !icon ) {
		p->fillRect( x, y, width, height, cg.dark() );
		return;
	}

	if ( placement() != PlaceInline && !QRect( xpos, ypos, width, height ).intersects( QRect( cx, cy, cw, ch ) ) )
		return;

#ifndef WIDGET_PLUGIN
	if ( placement() == PlaceInline )
		p->drawPixmap( x , y, icon->pixmap() );
	else
		p->drawPixmap( cx , cy, icon->pixmap(), cx - x, cy - y, cw, ch );
#endif
}

void TextIcon::iconUpdated(const QPixmap &)
{
	// TODO: any ideas what it should do? ;-)
}

QString TextIcon::richText() const
{
	QString s;
	s += "<icon ";
	QMap<QString, QString>::ConstIterator it = attributes.begin();
	for ( ; it != attributes.end(); ++it ) {
		s += it.key() + "=";
		if ( (*it).find( ' ' ) != -1 )
			s += "\"" + *it + "\"" + " ";
		else
			s += *it + " ";
	}
	s += ">";
	return s;
}

//----------------------------------------------------------------------------
// PsiStyleSheet - helper class for PsiTextView
//----------------------------------------------------------------------------

class PsiStyleSheet : public QStyleSheet
{
	Q_OBJECT
public:
	PsiStyleSheet(QWidget *parent = 0, const char *name = 0)
	: QStyleSheet(parent, name)
	{
		new QStyleSheetItem(this, QString::fromLatin1("icon"));
	}

	QTextCustomItem *tag(const QString &name, const QMap<QString, QString> &attr,
			const QString &context,
			const QMimeSourceFactory &factory,
			bool emptyTag, QTextDocument *doc ) const
	{
		const QStyleSheetItem *style = item( name );
		if ( style->name() == "icon" )
			return new TextIcon( doc, attr, context, (QMimeSourceFactory&)factory );
		return QStyleSheet::tag(name, attr, context, factory, emptyTag, doc);
	}
};

//----------------------------------------------------------------------------
// PsiTextView
//----------------------------------------------------------------------------

class PsiTextView::Private : public QObject
{
	Q_OBJECT

public:
	Private(QObject *parent)
	: QObject(parent, "PsiTextView::Private")
	{
		d = new URLObject(this);
	}

	URLObject *d;
};

PsiTextView::PsiTextView(QWidget *parent, const char *name)
: QTextEdit(parent, name)
{
	d = new Private(this);

	setReadOnly(true);
	setTextFormat(RichText);

	PsiStyleSheet *ss = new PsiStyleSheet(this, "PsiStyleSheet");
	setStyleSheet(ss);
}

QPopupMenu *PsiTextView::createPopupMenu(const QPoint &pos)
{
	QString link = anchorAt(pos);

	return d->d->createPopupMenu( link );
}

void PsiTextView::emitHighlighted(const QString &)
{
}

void PsiTextView::emitLinkClicked(const QString &s)
{
	d->d->link = s;
	d->d->popupAction();
}

//----------------------------------------------------------------------------
// URLLabel
//----------------------------------------------------------------------------

class URLLabel::Private : public QObject
{
	Q_OBJECT

public:
	Private()
	{
		d = new URLObject;
	}

	QString url;
	QString title;

	URLObject *d;
};

URLLabel::URLLabel(QWidget *parent, const char *name)
: QLabel(parent, name)
{
	d = new Private;
	setCursor( pointingHandCursor );
}

const QString &URLLabel::url() const
{
	return d->url;
}

void URLLabel::setUrl(const QString &url)
{
	d->url = url;
	updateText();
}

const QString &URLLabel::title() const
{
	return d->title;
}

void URLLabel::setTitle(const QString &t)
{
	d->title = t;
	updateText();
}

void URLLabel::updateText()
{
	setText( QString("<a href=\"%1\">%2</a>").arg(d->url).arg(d->title) );
}

void URLLabel::mouseReleaseEvent (QMouseEvent *e)
{
	QLabel::mouseReleaseEvent (e);

	switch ( e->button() ) {
	case LeftButton:
		break;

	case MidButton:
		break;

	case RightButton:
		{
			QPopupMenu *m = d->d->createPopupMenu( d->url );
			if ( m ) {
				m->exec( e->globalPos() );
				delete m;
			}
			break;
		}

	default:
		; // to supress warnings
	}
}

void URLLabel::enterEvent (QEvent *e)
{
	QLabel::enterEvent (e);
}

void URLLabel::leaveEvent (QEvent *e)
{
	QLabel::leaveEvent (e);
}

#include "psitextview.moc"
