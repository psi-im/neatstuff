/****************************************************************************
** iconset.cpp - various graphics handling classes
** Copyright (C) 2001, 2002, 2003  Justin Karneges
**                                 Michail Pishchagin
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,USA.
**
****************************************************************************/

#include"iconset.h"
#include"zip.h"

#include<qfile.h>
#include<qfileinfo.h>
#include<qtimer.h>

#include<qpixmap.h>
#include<qimage.h>
#include<qiconset.h>
#include<qregexp.h>
#include<qmime.h>
#include<qdom.h>
#include<qfile.h>
#include<qfileinfo.h>
#include<qcstring.h>
#include<qptrvector.h>

#include "anim.h"

//----------------------------------------------------------------------------
// Impix
//----------------------------------------------------------------------------

//! \class Impix iconset.h
//! \brief Combines a QPixmap and QImage into one class
//!
//!  Normally, it is common to use QPixmap for all application graphics.
//!  However, sometimes it is necessary to access pixel data, which means
//!  a time-costly conversion to QImage.  Impix does this conversion on
//!  construction, and keeps a copy of both a QPixmap and QImage for fast
//!  access to each.  What you gain in speed you pay in memory, as an Impix
//!  should occupy twice the space.
//!
//!  An Impix can be conveniently created from either a QImage or QPixmap
//!  source, and can be casted back to either type.
//!
//!  You may also call unload() to free the image data.
//!
//!  \code
//!  Impix i = QImage("icon.png");
//!  QLabel *iconLabel;
//!  iconLabel->setPixmap(i.pixmap());
//!  QImage image = i.image();
//!  \endcode

//! \if _hide_doc_
class Impix::Private : public QShared
{
public:
	Private()
	{
		pixmap = 0;
		image = 0;
	}

	~Private()
	{
		unload();
	}

	Private(const Private &from)
	: QShared()
	{
		pixmap = from.pixmap ? new QPixmap(*from.pixmap) : 0;
		image  = from.image  ? new QImage (*from.image)  : 0;
	}

	void unload()
	{
		delete pixmap;
		pixmap = 0;
		delete image;
		image = 0;
	}

	QPixmap *pixmap;
	QImage *image;
};
//! \endif

//! \brief Construct a null Impix
//!
//! Constructs an Impix without any image data.
Impix::Impix()
{
	d = new Private;
}

//! \brief Construct an Impix based on a QPixmap
//!
//! Constructs an Impix by making a copy of a QPixmap and creating a QImage from it.
//! \param from - source QPixmap
Impix::Impix(const QPixmap &from)
{
	d = new Private;
	*this = from;
}

//! \brief Construct an Impix based on a QImage
//!
//! Constructs an Impix by making a copy of a QImage and creating a QPixmap from it.
//! \param from - source QImage
Impix::Impix(const QImage &from)
{
	d = new Private;
	*this = from;
}

//!
//! \brief Copy constructor
Impix::Impix(const Impix &from)
{
	d = from.d;
	d->ref();
}

//!
//! Sets the Impix to \a from
Impix & Impix::operator=(const Impix &from)
{
	if ( d->deref() )
		delete d;

	d = from.d;
	d->ref();

	return *this;
}

//!
//! Unloads image data, making it null.
void Impix::unload()
{
	if ( isNull() )
		return;

	detach();
	d->unload();
}

//!
//! Destroys the image.
Impix::~Impix()
{
	if ( d->deref() )
		delete d;
}

bool Impix::isNull() const
{
	return d->image || d->pixmap ? false: true;
}

const QPixmap & Impix::pixmap() const
{
	if(!d->pixmap)
		d->pixmap = new QPixmap;
	return *d->pixmap;
}

const QImage & Impix::image() const
{
	if(!d->image)
		d->image = new QImage;
	return *d->image;
}

void Impix::setPixmap(const QPixmap &x)
{
	detach();

	d->unload();
	d->pixmap = new QPixmap(x);
	d->image = new QImage(x.convertToImage());
}

void Impix::setImage(const QImage &x)
{
	detach();

	d->unload();
	d->pixmap = new QPixmap;
	d->pixmap->convertFromImage(x);
	d->image = new QImage(x);
}

bool Impix::loadFromData(const QByteArray &ba)
{
	detach();

	QImage image;
	if ( image.loadFromData( ba ) ) {
		setImage ( image );
		return true;
	}
	return false;
}

Impix Impix::copy() const
{
	Impix impix;
	delete impix.d;
	impix.d = new Private( *this->d );

	return impix;
}

void Impix::detach()
{
	if ( d->count > 1 )
		*this = copy();
}

//----------------------------------------------------------------------------
// Icon
//----------------------------------------------------------------------------

/*!
	\class Icon
	\brief Can contain Anim and stuff

	This class can be used for storing application icons as well as emoticons
	(it has special functions in order to do that).

	Icons can contain animation, associated sound files, its own names.

	For implementing emoticon functionality, Icon can have associated text
	values and QRegExp for easy searching.
*/

//! \if _hide_doc_
class Icon::Private : public QShared
{
public:
	Private()
	{
		anim = 0;
		iconSet = 0;
		activatedCount = 0;
		text.setAutoDelete(true);
	}

	~Private()
	{
		unloadAnim();
		if ( iconSet )
			delete iconSet;
	}

	Private(const Private &from)
	: QShared()
	{
		name = from.name;
		regExp = from.regExp;
		text = from.text;
		sound = from.sound;
		impix = from.impix;
		anim = from.anim ? new Anim ( *from.anim ) : 0;
		iconSet = 0;
	}

	void unloadAnim()
	{
		if ( anim )
			delete anim;
		anim = 0;
	}

	QString name;
	QRegExp regExp;
	QDict<QString> text;
	QString sound;

	Impix impix;
	Anim *anim;
	QIconSet *iconSet;

	int activatedCount;
};
//! \endif

//!
//! Constructs empty Icon.
Icon::Icon()
: QObject(0, 0)
{
	d = new Private;
}

//!
//! Destroys Icon.
Icon::~Icon()
{
	if ( d->deref() )
		delete d;
}

//!
//! Creates new icon, that is a copy of \a from. Note, that if one icon will be changed,
//! other will be changed as well. (that's because image data is shared)
Icon::Icon(const Icon &from)
: QObject(0, 0)
{
	d = from.d;
	d->ref();
}

//!
//! Creates new icon, that is a copy of \a from. Note, that if one icon will be changed,
//! other will be changed as well. (that's because image data is shared)
Icon & Icon::operator= (const Icon &from)
{
	if ( d->deref() )
		delete d;

	d = from.d;
	d->ref();

	return *this;
}

Icon Icon::copy() const
{
	Icon icon;
	delete icon.d;
	icon.d = new Private( *this->d );

	return icon;
}

void Icon::detach()
{
	if ( d->count != 1 ) // only if >1 reference
		*this = copy();
}

//!
//! Returns \c true when icon contains animation.
bool Icon::isAnimated() const
{
	return d->anim != 0;
}

//!
//! Returns QPixmap of current frame.
const QPixmap &Icon::pixmap() const
{
	if ( d->anim )
		return d->anim->framePixmap();
	return d->impix.pixmap();
}

//!
//! Returns QImage of current frame.
const QImage &Icon::image() const
{
	if ( d->anim )
		return d->anim->frameImage();
	return d->impix.image();
}

//!
//! Returns Impix of first animation frame.
//! \sa setImpix()
const Impix &Icon::impix() const
{
	return d->impix;
}

//!
//! Returns QIconSet of first animation frame.
//! TODO: Add automatic greyscale icon generation.
const QIconSet &Icon::iconSet() const
{
	if ( d->iconSet )
		return *d->iconSet;
	
	d->iconSet = new QIconSet( d->impix.pixmap() );
	return *d->iconSet;
}

//!
//! Sets the Icon impix to \a impix.
//! \sa impix()
void Icon::setImpix(const Impix &impix)
{
	detach();

	d->impix = impix;
	emit pixmapChanged( pixmap() );
}

//!
//! Returns pointer to Anim object, or \a 0 if Icon doesn't contain an animation.
const Anim *Icon::anim() const
{
	return d->anim;
}

//!
//! Sets the animation for icon to \a anim. Also sets Impix to be the first frame of animation.
//! If animation have less than two frames, it is deleted.
//! \sa anim()
void Icon::setAnim(const Anim &anim)
{
	detach();

	d->anim = new Anim(anim);

	if ( d->anim->numFrames() > 0 )
		setImpix( d->anim->frame(0) );

	if ( d->anim->numFrames() < 2 ) {
		delete d->anim;
		d->anim = 0;
	}

	emit pixmapChanged( pixmap() );
}

//!
//! Returns name of the Icon.
//! \sa setName()
const QString &Icon::name() const
{
	return d->name;
}

//!
//! Sets the Icon name to \a name
//! \sa name()
void Icon::setName(const QString &name)
{
	detach();

	d->name = name;
}

//!
//! Returns Icon's QRegExp. It is used to store information for emoticons.
//! \sa setRegExp()
const QRegExp &Icon::regExp() const
{
	return d->regExp;
}

//!
//! Sets the Icon QRegExp to \a regExp.
//! \sa regExp()
//! \sa text()
void Icon::setRegExp(const QRegExp &regExp)
{
	detach();

	d->regExp = regExp;
}

//!
//! Returns Icon's text. It is used to store information for emoticons.
//! \sa setText()
//! \sa regExp()
const QDict<QString> &Icon::text() const
{
	return d->text;
}

//!
//! Sets the Icon text to \a t.
//! \sa text()
void Icon::setText(const QDict<QString> &t)
{
	detach();

	d->text = t;
}

//!
//! Returns file name of associated sound.
//! \sa setSound()
//! \sa activated()
const QString &Icon::sound() const
{
	return d->sound;
}

//!
//! Sets the sound file name to be associated with this Icon.
//! \sa sound()
//! \sa activated()
void Icon::setSound(const QString &sound)
{
	detach();

	d->sound = sound;
}

//!
//! Initializes Icon's Impix (or Anim, if \a isAnim equals \c true).
//! Iconset::load uses this function.
bool Icon::loadFromData(const QByteArray &ba, bool isAnim)
{
	detach();

	bool ret = false;
	if ( isAnim ) {
		Anim *anim = new Anim(ba);
		setAnim(*anim);
		delete anim; // shared data rules ;)
		ret = true;
	}

	if ( !ret && d->impix.loadFromData(ba) )
		ret = true;

	if ( ret )
		emit pixmapChanged( pixmap() );

	return ret;
}

//!
//! You need to call this function, when Icon is \e triggered, i.e. it is shown on screen
//! and it must start animation (if it has not animation, this function will do nothing).
//! When icon is no longer shown on screen you MUST call stop().
//! NOTE: For EACH activated() function call there must be associated stop() call, or the
//! animation will go crazy. You've been warned.
//! If \a playSound equals \c true, Icon will play associated sound file.
//! \sa stop()
void Icon::activated(bool playSound)
{
	d->activatedCount++;
	//qWarning("%-25s Icon::activated(): count = %d", name().latin1(), d->activatedCount);

	if ( playSound && !d->sound.isNull() ) {
		// TODO: insert sound playing code
	}

	if ( d->anim ) {
		d->anim->unpause();

		d->anim->disconnectUpdate (this, SLOT(animUpdate())); // ensure, that we're connected to signal exactly one time
		d->anim->connectUpdate (this, SLOT(animUpdate()));
	}
}

//!
//! You need to call this function when Icon is no more shown on screen. It would save
//! processor time, if Icon has animation.
//! NOTE: For EACH activated() function call there must be associated stop() call, or the
//! animation will go crazy. You've been warned.
//! \sa activated()
void Icon::stop()
{
	d->activatedCount--;
	//qWarning("%-25s Icon::stop(): count = %d", name().latin1(), d->activatedCount);

	if ( d->activatedCount <= 0 ) {
		d->activatedCount = 0;
		if ( d->anim ) {
			d->anim->pause();
			d->anim->restart();
		}
	}
}

//! \internal
void Icon::animUpdate()
{
	emit pixmapChanged( pixmap() );
}

//----------------------------------------------------------------------------
// IconsetFactory
//----------------------------------------------------------------------------

/*!
	\class IconsetFactory
	\brief Class for easy application-wide Icon searching

	You can add several Iconsets to IconsetFactory to use multiple Icons
	application-wide with ease.

	You can reference Icons by their name from any place from your application.
*/

//! \if _hide_doc_
class IconsetFactoryPrivate
{
private:
	static QPtrVector<Iconset> *iconsets;

public:
	static void registerIconset(const Iconset *);
	static void unregisterIconset(const Iconset *);

public:
	static const Icon *icon(const QString &name);

	friend class IconsetFactory;
};
//! \endif

QPtrVector<Iconset> *IconsetFactoryPrivate::iconsets = 0;

void IconsetFactoryPrivate::registerIconset(const Iconset *i)
{
	if ( !iconsets || iconsets->find(i) < 0 ) {
		if ( !iconsets )
			iconsets = new QPtrVector<Iconset>;

		iconsets->resize ( iconsets->count() + 1 );
		iconsets->insert ( iconsets->count(), i );
	}
}

void IconsetFactoryPrivate::unregisterIconset(const Iconset *i)
{
	if ( iconsets && iconsets->find(i) >= 0 ) {
		iconsets->remove ( iconsets->find(i) );

		if ( !iconsets->count() ) {
			delete iconsets;
			iconsets = 0;
		}
	}
}

const Icon *IconsetFactoryPrivate::icon(const QString &name)
{
	if ( !iconsets )
		return 0;

	const Icon *i = 0;
	for (uint j = 0; j < iconsets->count(); j++) {
		i = iconsets->at(j)->icon(name);
		if ( i )
			break;
	}
	return i;
}

//!
//! Returns pointer to Icon with name \a name, or \a 0 if Icon with that name wasn't
//! found in IconsetFactory.
const Icon *IconsetFactory::iconPtr(const QString &name)
{
	const Icon *i = IconsetFactoryPrivate::icon(name);
	if ( !i ) {
		qDebug("WARNING: IconsetFactory::icon(\"%s\"): icon not found", name.latin1());
	}
	return i;
}

//!
//! Returns Icon with name \a name, or empty Icon if Icon with that name wasn't
//! found in IconsetFactory.
const Icon IconsetFactory::icon(const QString &name)
{
	const Icon *i = iconPtr(name);
	if ( i )
		return *i;
	return Icon();
}

//!
//! Returns list of all Icon names that are in IconsetFactory.
const QStringList IconsetFactory::icons()
{
	QStringList list;

	QPtrVector<Iconset> *iconsets = IconsetFactoryPrivate::iconsets;
	uint count = 0;
	if ( iconsets )
		count = iconsets->count();
	for (uint i = 0; i < count; i++) {
		QDictIterator<Icon> it = iconsets->at(i)->iterator();
		for ( ; it.current(); ++it)
			list << (*it)->name();
	}

	return list;

}

//----------------------------------------------------------------------------
// Iconset
//----------------------------------------------------------------------------

/*!
	\class Iconset
	\brief Class for easy Icon grouping

	This class supports loading Icons from .zip arhives. It also provides additional
	information: name(), authors(), version(), description() and creation() date.

	This class is very handy to manage emoticon sets as well as usual application icons.

	\sa IconsetFactory
*/

//! \if _hide_doc_
class Iconset::Private : public QShared
{
private:
	void init()
	{
		name = "Unnamed";
		version = "1.0";
		description = "No description";
		authors << "I. M. Anonymous";
		creation = "XXXX-XX-XX";
		homeUrl = QString::null;

		list.setAutoDelete(true);
	}

public:
	QString name, version, description, creation, homeUrl, filename;
	QStringList authors;
	QDict<Icon> list;
	QDict<QString> info;
	//Icon nullIcon;

public:
	Private()
	{
		init();
	}

	Private(const Private &from)
	: QShared()
	{
		init();

		name = from.name;
		version = from.version;
		description = from.description;
		creation = from.creation;
		homeUrl = from.homeUrl;
		filename = from.filename;
		authors = from.authors;
		info = from.info;

		QDictIterator<Icon> it( from.list );
		for ( ; it.current(); ++it)
			list.insert(it.currentKey(), new Icon(*it.current()));
	}

	QByteArray loadData(const QString &fileName, const QString &dir)
	{
		QByteArray ba;

		QFileInfo fi(dir);
		if ( fi.isDir() ) {
			QFile file ( dir + "/" + fileName );
			file.open (IO_ReadOnly);

			ba = file.readAll();
		}
		else if ( fi.extension(false) == "jisp" || fi.extension(false) == "zip" ) {
			UnZip z(dir);
			if ( !z.open() )
				return ba;

			QString name = fi.baseName(true) + "/" + fileName;
			if ( !z.readFile(name, &ba) ) {
				name = "/" + fileName;
				z.readFile(name, &ba);
			}
		}

		return ba;
	}

	void loadMeta(const QDomElement &i, const QString &dir)
	{
		for (QDomNode node = i.firstChild(); !node.isNull(); node = node.nextSibling()) {
			QDomElement e = node.toElement();
			if( e.isNull() )
				continue;

			QString tag = e.tagName();
			if ( tag == "name" ) {
				name = e.text();
			}
			else if ( tag == "version" ) {
				version = e.text();
			}
			else if ( tag == "description" ) {
				description = e.text();
			}
			else if ( tag == "author" ) {
				QString name = e.text();
				if ( !e.attribute("email").isEmpty() )
					name = QString("%1<br>&nbsp;&nbsp;Email: %2").arg(name).arg( e.attribute("email") );
				if ( !e.attribute("jid").isEmpty() )
					name = QString("%1<br>&nbsp;&nbsp;JID: %2").arg(name).arg( e.attribute("jid") );
				if ( !e.attribute("www").isEmpty() )
					name = QString("%1<br>&nbsp;&nbsp;WWW: %2").arg(name).arg( e.attribute("www") );
				authors += name;
			}
			else if ( tag == "creation" ) {
				creation = e.text();
			}
			else if ( tag == "home" ) {
				homeUrl = e.text();
			}
		}
	}

	static int icon_counter; // used to give unique names to icons

	void loadIcon(const QDomElement &i, const QString &dir)
	{
		Icon icon;

		QDict<QString> text, graphic, sound, object;
		graphic.setAutoDelete(true);
		sound.setAutoDelete(true);
		object.setAutoDelete(true);

		QString name;
		name.sprintf("icon_%04d", icon_counter++);
		bool isAnimated = false;
		bool isImage = false;

		for(QDomNode n = i.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if ( e.isNull() )
				continue;

			QString tag = e.tagName();
			if ( tag == "text" ) {
				QString lang = e.attribute("xml:lang");
				if ( lang.isEmpty() )
					lang = ""; // otherwise there would be many warnings :-(
				text.insert( lang, new QString(e.text()));
			}
			else if ( tag == "object" ) {
				object.insert( e.attribute("mime"), new QString(e.text()));
			}
			else if ( tag == "x" ) {
				QString attr = e.attribute("xmlns");
				if ( attr == "name" ) {
					name = e.text();
				}
				else if ( attr == "type" ) {
					if ( e.text() == "animation" )
						isAnimated = true;
					else if ( e.text() == "image" )
						isImage = true;
				}
			}
			// leaved for compatibility with old JEP
			else if ( tag == "graphic" ) {
				graphic.insert( e.attribute("mime"), new QString(e.text()));
			}
			else if ( tag == "sound" ) {
				sound.insert( e.attribute("mime"), new QString(e.text()));
			}
		}

		icon.setText (text);
		icon.setName (name);

		QStringList graphicMime, soundMime, animationMime;
		graphicMime << "image/png"; // first item have higher priority than latter
		graphicMime << "video/x-mng";
		graphicMime << "image/gif";
		// TODO: untested
		graphicMime << "image/bmp";
		graphicMime << "image/x-xpm";
		graphicMime << "image/svg+xml";
		graphicMime << "image/jpeg";

		// TODO: needs testing
		soundMime << "audio/x-wav";
		soundMime << "audio/x-ogg";
		soundMime << "audio/x-mp3";
		soundMime << "audio/x-midi";

		// MIME-types, that support animations
		animationMime << "image/gif";
		//animationMime << "image/png";
		animationMime << "image/x-mng";

		if ( !object.isEmpty() ) {
			// fill the graphic & sound tables, if there are some
			// 'object' entries. inspect the supported mimetypes
			// and copy mime info and file path to 'graphic' and
			// 'sound' dictonaries.

			QStringList::Iterator it = graphicMime.begin();
			for ( ; it != graphicMime.end(); ++it)
				if ( object[*it] && !object[*it]->isNull() )
					graphic.insert( *it, new QString(*object[*it]));

			it = soundMime.begin();
			for ( ; it != soundMime.end(); ++it)
				if ( object[*it] && !object[*it]->isNull() )
					sound.insert( *it, new QString(*object[*it]));
		}

		{
			QStringList::Iterator it = graphicMime.begin();
			for ( ; it != graphicMime.end(); ++it) {
				if ( graphic[*it] && !graphic[*it]->isNull() ) {
					// if format supports animations, then load graphic as animation, and
					// if there is only one frame, then later it would be converted to single Impix
					if ( !isAnimated && !isImage ) {
						QStringList::Iterator it2 = animationMime.begin();
						for ( ; it2 != animationMime.end(); ++it2)
							if ( *it == *it2 ) {
								isAnimated = true;
								break;
							}
					}

					if ( icon.loadFromData( loadData(*graphic[*it], dir), isAnimated ) )
						break;
				}
			}
		}

		{
			QFileInfo fi(dir);
			QStringList::Iterator it = soundMime.begin();
			for ( ; it != soundMime.end(); ++it) {
				if ( sound[*it] && !sound[*it]->isNull() ) {
					if ( !fi.isDir() ) { // it is a .zip then
						// TODO: write unpacking code here
						// probably it should create directory with
						// archive name in the data directory, and
						// unpack there...
					}
					else {
						icon.setSound ( dir + "/" + *sound[*it] );
						break;
					}
				}
			}
		}

		if ( text.count() )
		{	// construct RegExp
			QString regexp;
			QDictIterator<QString> it( text );
			for ( ; it.current(); ++it ) {
				if ( !regexp.isEmpty() )
					regexp += '|';

				regexp += QRegExp::escape(**it);
			}

			// make sure there is some form of whitespace on at least one side of the text string
			regexp = QString("(\\b(%1))|((%2)\\b)").arg(regexp).arg(regexp);
			icon.setRegExp ( regexp );
		}

		list.replace (name, new Icon(icon));
	}

	bool load(const QDomDocument &doc, const QString dir)
	{
		QDomElement base = doc.documentElement();
		if ( base.tagName() != "icondef" )
			return false;

		for(QDomNode node = base.firstChild(); !node.isNull(); node = node.nextSibling()) {
			QDomElement i = node.toElement();
			if( i.isNull() )
				continue;

			QString tag = i.tagName();
			if ( tag == "meta" ) {
				loadMeta (i, dir);
			}
			else if ( tag == "icon" ) {
				loadIcon (i, dir);
			}
			else if ( tag == "x" ) {
				info.insert( i.attribute("xmlns"), new QString(i.text()) );
			}
		}

		return true;
	}
};
//! \endif

int Iconset::Private::icon_counter = 0;

//!
//! Creates empty Iconset.
Iconset::Iconset()
{
	d = new Private;
}

//!
//! Creates shared copy of Iconset \a from.
Iconset::Iconset(const Iconset &from)
{
	d = from.d;
	d->ref();
}

//!
//! Destroys Iconset, and frees all allocated Icons.
Iconset::~Iconset()
{
	if ( d->deref() )
		delete d;

	IconsetFactoryPrivate::unregisterIconset(this);
}

//!
//! Copies all Icons as well as additional information from Iconset \a from.
Iconset &Iconset::operator=(const Iconset &from)
{
	if ( d->deref() )
		delete d;

	d = from.d;
	d->ref();

	return *this;
}

Iconset Iconset::copy() const
{
	Iconset is;
	delete is.d;
	is.d = new Private( *this->d );

	return is;
}

void Iconset::detach()
{
	if ( d->count > 1 )
		*this = copy();
}

//!
//! Appends icons from Iconset \a from to this Iconset.
Iconset &Iconset::operator+=(const Iconset &i)
{
	detach();

	QDictIterator<Icon> it( i.d->list );
	for ( ; it.current(); ++it)
		d->list.insert(it.currentKey(), new Icon(*it.current()));
	return *this;
}

//!
//! Frees all allocated Icons.
void Iconset::clear()
{
	detach();

	d->list.clear();
}

//!
//! Returns the number of Icons in Iconset.
uint Iconset::count() const
{
	return d->list.count();
}

//!
//! Loads Icons and additional information from directory \a dir. Directory can usual directory,
//! or a .zip/.jisp archive. There must exist file named \c icondef.xml in that directory.
bool Iconset::load(const QString &dir)
{
	detach();

	QByteArray ba;
	ba = d->loadData ("icondef.xml", dir);
	if ( !ba.isEmpty() ) {
		QDomDocument doc;
		if ( doc.setContent(ba, false) ) {
			if ( d->load(doc, dir) ) {
				d->filename = dir;
				return true;
			}
		}
		else
			qWarning("Iconset::load(): Failed to load iconset: icondef.xml is invalid XML");
	}
	return false;
}

//!
//! Returns pointer to Icon, if Icon with name \a name was found in Iconset, or \a 0 otherwise.
//! \sa setIcon()
const Icon *Iconset::icon(const QString &name) const
{
	if ( d->list.isEmpty() )
		return 0;

	return d->list.find(name);
}

//!
//! Appends (or replaces, if Icon with that name already exists) Icon to Iconset.
void Iconset::setIcon(const QString &name, const Icon &icon)
{
	detach();

	d->list.replace (name, new Icon(icon));
}

//!
//! Removes Icon with the name \a name from Iconset.
void Iconset::removeIcon(const QString &name)
{
	detach();

	d->list.remove (name);
}

//!
//! Returns the Iconset name.
const QString &Iconset::name() const
{
	return d->name;
}

//!
//! Returns the Iconset version.
const QString &Iconset::version() const
{
	return d->version;
}

//!
//! Returns the Iconset description string.
const QString &Iconset::description() const
{
	return d->description;
}

//!
//! Returns the Iconset authors list.
const QStringList &Iconset::authors() const
{
	return d->authors;
}

//!
//! Returns the Iconset creation date.
const QString &Iconset::creation() const
{
	return d->creation;
}

//!
//! Returns the Iconsets' home URL.
const QString &Iconset::homeUrl() const
{
	return d->homeUrl;
}

QDictIterator<Icon> Iconset::iterator() const
{
	//detach(); // ???

	QDictIterator<Icon> it( d->list );
	return it;
}

//!
//! Returns directory (or .zip/.jisp archive) name from which Iconset was loaded.
const QString &Iconset::fileName() const
{
	return d->filename;
}

//!
//! Sets the Iconset directory (.zip archive) name.
void Iconset::setFileName(const QString &f)
{
	d->filename = f;
}

//!
//! Returns additional Iconset information.
//! \sa setInfo()
const QDict<QString> Iconset::info() const
{
	return d->info;
}

//!
//! Sets additional Iconset information.
//! \sa info()
void Iconset::setInfo(const QDict<QString> &i)
{
	d->info = i;
}

//!
//! Created QMimeSourceFactory with names of icons and their images.
QMimeSourceFactory *Iconset::createMimeSourceFactory() const
{
	QMimeSourceFactory *m = new QMimeSourceFactory;

	QDictIterator<Icon> it( d->list );
	for ( ; it.current(); ++it)
		m->setImage(it.currentKey(), it.current()->image());

	return m;
}

//!
//! Adds Iconset to IconsetFactory.
void Iconset::addToFactory() const
{
	IconsetFactoryPrivate::registerIconset(this);
}

//!
//! Removes Iconset from IconsetFactory.
void Iconset::removeFromFactory() const
{
	IconsetFactoryPrivate::unregisterIconset(this);
}
