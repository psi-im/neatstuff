/****************************************************************************
** iconset.h - various graphics handling classes
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

#ifndef ICONSET_H
#define ICONSET_H

#include <qobject.h>
#include <qdict.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>

class QImage;
class QPixmap;
class QMimeSourceFactory;
class Anim;

class Impix
{
public:
	Impix();
	Impix(const QPixmap &);
	Impix(const QImage &);
	Impix(const Impix &);
	Impix & operator=(const Impix &from);
	~Impix();

	void unload();
	bool isNull() const;

	const QPixmap & pixmap() const;
	const QImage & image() const;
	void setPixmap(const QPixmap &);
	void setImage(const QImage &);

	operator const QPixmap &() const { return pixmap(); }
	operator const QImage &() const { return image(); }
	Impix & operator=(const QPixmap &from) { setPixmap(from); return *this; }
	Impix & operator=(const QImage &from) { setImage(from); return *this; }
	
	bool loadFromData(const QByteArray &);

private:
	class Private;
	Private *d;
};

class Icon : public QObject
{
	Q_OBJECT
public:
	Icon();
	Icon(const Icon &);
	~Icon();
	
	Icon & operator= (const Icon &);

	//!
	//! see pixmap().
	operator const QPixmap &() const { return pixmap(); }

	//!
	//! see image().
	operator const QImage &() const { return image(); }
	
	bool isAnimated() const;
	const QPixmap &pixmap() const;
	const QImage &image() const;
	
	const Impix &impix() const;
	void setImpix(const Impix &);
	
	const Anim *anim() const;
	void setAnim(const Anim &);
	
	const QString &name() const;
	void setName(const QString &);
	
	const QRegExp &regExp() const;
	void setRegExp(const QRegExp &);
	
	const QDict<QString> &text() const;
	void setText(const QDict<QString> &);
	
	const QString &sound() const;
	void setSound(const QString &);
	
	bool loadFromData(const QByteArray &, bool isAnimation);
	
signals:
	void pixmapChanged(const QPixmap &);

public slots:
	void activated(bool playSound = true);	// it just has been inserted in the text, or now it's being displayed by
						// some widget. icon should play sound and start animation
	
	void stop();	// this icon is no more displaying. stop animation
	
private slots:
	void animUpdate();
	
private:
	class Private;
	Private *d;
};

class Iconset
{
public:
	Iconset();
	Iconset(const Iconset &);
	~Iconset();

	Iconset &operator=(const Iconset &);
	Iconset &operator+=(const Iconset &);
	
	void clear();
	uint count() const;
	
	bool load(const QString &dir);
	
	const Icon *icon(const QString &) const;
	void setIcon(const QString &, const Icon &);
	void removeIcon(const QString &);

	const QString &name() const;
	const QString &version() const;
	const QString &description() const;
	const QStringList &authors() const;
	const QString &creation() const;

	const QString &fileName() const;
	void setFileName(const QString &);
	
	const QDict<QString> info() const;
	void setInfo(const QDict<QString> &);
	
	QDictIterator<Icon> iterator() const;
	
	QMimeSourceFactory *createMimeSourceFactory() const;

	void addToFactory() const;
	void removeFromFactory() const;
	
private:
	class Private;
	Private *d;
};

class IconsetFactory
{
public:
	static const Icon &icon(const QString &name);
	static const Icon *iconPtr(const QString &name);
	static const QStringList icons();
};

#endif
