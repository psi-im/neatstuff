#include "fancypopup.h"

#include <qpixmap.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qptrlist.h>

#include "fancylabel.h"

//----------------------------------------------------------------------------
// FancyPopup::Private
//----------------------------------------------------------------------------

class FancyPopup::Private : public QObject
{
	Q_OBJECT
public:
	Private(FancyPopup *p);
	~Private();

	QRegion maskRegion();
	void paintBackround();

signals:
	void animating();

public slots:
	void hide();
	void anim();
	void draw();
	void popupDestroyed(QObject *);

public:
	// parameters
	static const int radius = 15;
	static const bool animEnabled = true;
	static const int animSpeed = 15;
	static const int animTimeout = 50;
	static const int hideTimeout = 5 * 1000; // 5 seconds

	// animation
	enum State {
		State_Opening,
		State_Open,
		State_Closing
	};
	State state;
	int animH;

	// misc
	FancyPopup *popup;
	QTimer *hideTimer, *animTimer;
	QPixmap background;
	QRegion backgroundMask;
	QPtrList<FancyPopup> popupList;
};

FancyPopup::Private::Private(FancyPopup *p)
: QObject(p)
{
	popup = p;

	hideTimer = new QTimer(this);
	connect(hideTimer, SIGNAL(timeout()), SLOT(hide()));
	connect(popup,     SIGNAL(clicked()), SLOT(hide()));

	// animation
	animTimer = new QTimer(this);
	connect(animTimer, SIGNAL(timeout()), SLOT(anim()));

	if ( animEnabled ) {
		state = State_Opening;
		animH = 0;

		animTimer->start(animTimeout);
	}
	else {
		state = State_Open;
		animH = 9999;
	}
}

FancyPopup::Private::~Private()
{
}

void FancyPopup::Private::hide()
{
	if ( animEnabled ) {
		state = State_Closing;
		animTimer->start(animTimeout);
	}
	else {
		popup->hide();
	}
}

void FancyPopup::Private::anim()
{
	bool update = false;

	if ( state == State_Opening ) {
		animH += animSpeed;
		update = true;

		if ( animH >= (popup->height() - radius) ) {
			animH = popup->height() - radius;

			state = State_Open;
			animTimer->stop();
		}
	}
	else if ( state == State_Closing ) {
		animH -= animSpeed;
		update = true;

		if ( animH <= 0 ) {
			popup->hide();
		}
	}

	if ( update )
		draw();
}

void FancyPopup::Private::draw()
{
	if ( state == State_Open && !hideTimer->isActive() && hideTimeout != 0 )
		hideTimer->start(hideTimeout);

	if ( animH > (popup->height() - radius) )
		animH = popup->height() - radius;

	emit animating();

	QRegion r = backgroundMask;

	r -= QRegion (0, animH, popup->width(), popup->height());

	int x = qApp->desktop()->width() - popup->width() - 25;
	QRect geom = qApp->desktop()->availableGeometry(popup);
	int y = geom.y() + geom.height(); //qApp->desktop()->height();

	int popH = 0;
	QPtrListIterator<FancyPopup> it(popupList);
	FancyPopup *pop;
	while ( (pop = it.current()) ) {
		popH += pop->d->animH;
		++it;
	}

	if ( popH ) {
		int delta = popH - radius;
		r += QRegion(0, popup->height() - radius, popup->width(), popup->height() - radius + delta);
	}

	popup->move ( x, y - popH - animH );
	popup->setMask(r);
}

void FancyPopup::Private::popupDestroyed(QObject *obj)
{
	popupList.remove((FancyPopup *)obj);
	draw();
}

QRegion FancyPopup::Private::maskRegion()
{
	const int r = Private::radius;
	const int w = popup->width(), h = popup->height() - r;
	QRegion mask(0, 0, w, h);

	QPoint corners[4] = {
		QPoint(w - r*2, 0),
		QPoint(0, 0),

		QPoint(w, 0),
		QPoint(0, 0)
	};

	for (int i = 0; i < 2; i++) {
		QPointArray corner;
		corner.makeArc  (corners[i].x(), corners[i].y(), r*2, r*2, i * 90*16, 90*16);
		corner.resize   (corner.size() + 1);
		corner.setPoint (corner.size() - 1, corners[i + 2]);
		mask -= corner;
	}

	/*{
	QPointArray corner;
	corner.makeArc  (width() - r*2, 0, r*2, r*2, 0, 90*16);
	corner.resize   (corner.size() + 1);
	corner.setPoint (corner.size() - 1, QPoint(width(), 0));
	mask -= corner;
	}

	{
	QPointArray corner;
	corner.makeArc  (0, 0, r*2, r*2, 90*16, 90*16);
	corner.resize   (corner.size() + 1);
	corner.setPoint (corner.size() - 1, QPoint(0, 0));
	mask -= corner;
	}*/

/*        QPoint corners[8] = {
		QPoint(width()-40, 0),
		QPoint(0, 0),
		QPoint(0, height()-40),
		QPoint(width()-40, height()-40),
                QPoint(width(), 0),
		QPoint(0, 0),
		QPoint(0, height()),
		QPoint(width(), height())
	};
	for (int i = 0; i < 2; ++i)
	{
		QPointArray corner;
		corner.makeArc(corners[i].x(), corners[i].y(), 40, 40, i * 16 * 90, 16 * 90);
		corner.resize(corner.size() + 1);
		corner.setPoint(corner.size() - 1, corners[i + 4]);
		mask -= corner;
	}*/
/*	bool bottom, right;
	QDesktopWidget* tmp = QApplication::desktop();
	QRect deskRect;
	// get screen-geometry for screen our anchor is on
	// (geometry can differ from screen to screen!
	deskRect = tmp->screenGeometry( tmp->screenNumber(this->pos()) );
	bottom = this->pos().y() + height() > deskRect.height() - 48;
	right = this->pos().x() + width() > deskRect.width() - 48;
*/
	/*QPointArray arrow(4);
	arrow.setPoint(0, QPoint(right ? width() : 0, bottom ? height() : 0));
	arrow.setPoint(1, QPoint(right ? width() - 10 : 10, bottom ? height() - 30 : 30));
	arrow.setPoint(2, QPoint(right ? width() - 30 : 30, bottom ? height() - 10 : 10));
	arrow.setPoint(3, arrow[0]);
	mask += arrow;*/
	//setMask(mask);
	return mask;
}

void FancyPopup::Private::paintBackround()
{
	QRect rect = popup->contentsRect();
	if ( rect.width() <= 0 || rect.height() <= 0 )
		return; // avoid crash

	// first, draw nice top-down gradient
	QColor from(240, 245, 250), to(130, 171, 213);

	int r1, g1, b1;
	from.rgb (&r1, &g1, &b1);
	int r2, g2, b2;
	to.rgb (&r2, &g2, &b2);

	int h = 60; // gradient height
	float stepR = (float)(r2 - r1) / h;
	float stepG = (float)(g2 - g1) / h;
	float stepB = (float)(b2 - b1) / h;

	QPixmap pix (rect.width(), rect.height());
	QPainter p;
	p.begin (&pix);
	for (int i = 0; i < h; i++) {
		int r = (int)((float)r1 + stepR*i);
		int g = (int)((float)g1 + stepG*i);
		int b = (int)((float)b1 + stepB*i);

		p.setPen ( QColor( r, g, b ) );
		p.drawLine ( 0, i, rect.width(), i );
	}

	p.fillRect(0, h, rect.width(), rect.height(), to);

	// now, the borders
	p.setPen( QColor(10, 10, 10) );
	p.moveTo(0,0);
	p.lineTo(rect.width()-1, 0);
	p.lineTo(rect.width()-1, rect.height()-1);
	p.lineTo(0, rect.height()-1);
	p.lineTo(0,0);

	const int r = radius;
	p.drawArc(1, 1, r*2, r*2, 90 * 16, 90 * 16);
	p.drawArc(rect.width() - r*2 - 1, 1, r*2, r*2, 0, 90 * 16);

	p.end ();

	/*QObjectList *l = queryList( "QLabel" );
	QObjectListIt it( *l );
	while ( it.current() ) {
		QLabel *child = (QLabel *)it.current();
		child->update();

		++it;
	}
	delete l;*/

	popup->setUpdatesEnabled(false);
	popup->setPaletteBackgroundPixmap(pix);
	popup->setUpdatesEnabled(true);

	background = pix;
}

//----------------------------------------------------------------------------
// FancyPopup
//----------------------------------------------------------------------------

//static const int DEFAULT_POPUP_TIME = 6*1000;
static const int POPUP_FLAGS = Qt::WStyle_Customize | Qt::WDestructiveClose | Qt::WX11BypassWM
                             | Qt::WStyle_StaysOnTop | Qt::WStyle_Tool | Qt::WStyle_NoBorder;

FancyPopup::FancyPopup(FancyPopup *prev, QString titleStr, QString titleIconStr, QString statusIconStr,
QString nameStr, QString JID, QString statusStr)
: QFrame( 0, 0, POPUP_FLAGS | WRepaintNoErase | WResizeNoErase )
{
	d = new Private(this);

	QVBoxLayout *vbox = new QVBoxLayout(this, 5, 3);
	QHBoxLayout *topBox = new QHBoxLayout(0, 0, 3);

	topBox->addSpacing(Private::radius);

	IconLabel *titleIcon = new IconLabel(this);
	titleIcon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	titleIcon->setIcon(titleIconStr);
	topBox->addWidget(titleIcon);

	QLabel *title = new QLabel(QString("<div align=\"center\"><h3>%1</h3></div>").arg(titleStr), this);
	title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	topBox->addWidget(title);

	topBox->addSpacing(Private::radius);
	vbox->addLayout(topBox);

	QHBoxLayout *dataBox = new QHBoxLayout(0, 0, 5);

	IconLabel *status = new IconLabel(this);
	status->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	status->setIcon(statusIconStr);
	dataBox->addWidget(status);

	QLabel *info = new QLabel(this);
	title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	info->setText(QString("<qt>%1&nbsp;<i>&lt;%2&gt;</i><br><small>%3</small></qt>").arg(nameStr).arg(JID).arg(statusStr));
	dataBox->addWidget(info);

	vbox->addLayout(dataBox);
	vbox->addSpacing(Private::radius);

	setMinimumWidth  (Private::radius * 3);
	setMinimumHeight (Private::radius * 2);

	if ( prev ) {
		if ( prev->width() > sizeHint().width() )
			setMinimumWidth( prev->width() );
	}
}

FancyPopup::~FancyPopup()
{
}

void FancyPopup::show()
{
	if ( size() != sizeHint() )
		resize( sizeHint() );

	setPalette(QToolTip::palette());

	d->backgroundMask = d->maskRegion();
	updateMask();

	d->paintBackround();
	d->draw();
	QFrame::show();
}

void FancyPopup::hideEvent(QHideEvent *)
{
	d->hideTimer->stop();
	//if ( m_autoDelete )
		deleteLater();
}

void FancyPopup::mouseReleaseEvent(QMouseEvent *)
{
	emit clicked();
}

void FancyPopup::updateMask()
{
	setMask(d->backgroundMask);
}

void FancyPopup::drawContents(QPainter *painter)
{
	painter->drawPixmap (contentsRect().topLeft(), d->background);
}

void FancyPopup::prependPopup(const FancyPopup *p)
{
	d->popupList.append(p);
	connect(p->d, SIGNAL(animating()), d, SLOT(draw()));
	connect(p, SIGNAL(destroyed(QObject *)), d, SLOT(popupDestroyed(QObject *)));
}

#include "fancypopup.moc"
