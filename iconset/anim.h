/****************************************************************************
** anim.h - Anim -- class for handling animations
** Copyright (C) 2003  Michail Pishchagin
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

#ifndef ANIM_H
#define ANIM_H

#include <qcstring.h>

class QObject;
class QPixmap;
class QImage;
class Impix;

class Anim
{
public:
	Anim(const QByteArray &data);
	Anim(const Anim &anim);
	~Anim();
	
	const QPixmap &framePixmap() const;
	const QImage &frameImage() const;
	const Impix &frameImpix() const;
	bool isNull() const;
	
	int framenumber() const;
	int numFrames() const;
	const Impix &frame(int n) const;
	
	bool paused() const;
	void unpause();
	void pause();

	void restart();

	void connectUpdate(QObject *receiver, const char *member);
	void disconnectUpdate(QObject *receiver, const char *member = 0);

	Anim copy() const;
	void detach();

	class Private;
private:
	Private *d;
};

#endif
