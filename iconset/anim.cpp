/****************************************************************************
** anim.cpp - Anim -- class for handling animations
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

#include "anim.h"
#include "iconset.h"

#include <qobject.h>
#include <qasyncimageio.h>
#include <qptrvector.h>
#include <qtimer.h>

/*!
	\class Anim
	\brief Class for handling animations

	Anim is a class that can load animations. Generally, it looks like
	QMovie but it loads animations in one pass and stores it in memory.

	Each frame of Anim is stored as Impix.
*/

//! \if _hide_doc_
class Anim::Private : public QObject, public QShared, private QImageConsumer
{
	Q_OBJECT
public:
	QImageDecoder *decoder;
	QTimer *frametimer;

	bool empty;
	bool paused;

	int speed;
	int lasttimerinterval, frameperiod;

	int looping, loop;

	class Frame {
	public:
		Frame(Impix *i, int p)
		{
			impix = i;
			period = p;
		}
		~Frame()
		{
			delete impix;
		}
		
		Impix *impix;
		int period;
	};
	
	QPtrVector<Frame> frames;
	int frame;

public:
	void init()
	{
		frametimer = 0;
	
		speed = 100;
		lasttimerinterval = -1;
		frameperiod = 120;

		looping = 0; // MNG movies doesn't contain loop flag?
		loop = 0;
		frame = 0;
		paused = true;
	}

	Private()
	{
		init();
	}
	
	Private(const QByteArray ba)
	{
		init();
		
		frametimer = new QTimer(this);
		QObject::connect(frametimer, SIGNAL(timeout()), this, SLOT(refresh()));
		decoder = new QImageDecoder(this);
		
		uchar *buf = reinterpret_cast<uchar *>(ba.data());
		int bytecount = ba.count();
		while ( bytecount ) {
			// TODO: what number of cycles require MNG movies for normal display?
			for (int i = 0; i < 40; i++) { // the more is counter, more frames MNG animation will give :)
				int used = decoder->decode(buf, !i ? bytecount : 0);
				if ( used < 0 ) {
					qWarning("Anim::loadAnim: Unrecognized Format error");
					break;
				}
				buf += used;
				bytecount -= used;
			}
		}
	
		delete decoder;
		//qWarning("Anim:: %d frames", numFrames());		
	}
	
	~Private()
	{
		if ( frametimer )
			delete frametimer;
	}

	void pause()
	{
		paused = true;
		frametimer->stop();
	}
	
	void unpause()
	{
		paused = false;
		restartTimer();
	}
	
	void restart()
	{
		frame = 0;
		if ( !paused )
			restartTimer();
	}

	int numFrames() const
	{
		return frames.count();
	}
	
	void restartTimer()
	{
		if (!paused && speed > 0) {
			frameperiod = frames[frame]->period;
			int i = frameperiod >= 0 ? frameperiod * 100/speed : 0;
			if ( i != lasttimerinterval || !frametimer->isActive() ) {
				lasttimerinterval = i;
				frametimer->start( i );
			}
		} else {
			frametimer->stop();
		}
	}
	
	// QImageConsumer
	void end()
	{
	}

	void changed(const QRect &)
	{
	}

	void frameDone()
	{
		frames.resize ( frames.count()+1 );
		frames.insert ( frames.count(), new Frame(new Impix( decoder->image() ), frameperiod) );
	}

	void frameDone(const QPoint &, const QRect &)
	{
		frameDone();
	}

	void setLooping(int loop)
	{
		looping = loop;
	}

	void setFramePeriod(int period)
	{
		frameperiod = period;
	}

	void setSize(int, int)
	{
	}

signals:
	void areaChanged();

public slots:
	void refresh()
	{
		frame++;
		if ( frame >= numFrames() ) {
			frame = 0;

			loop++;
			if ( looping && loop >= looping ) {
				frame = numFrames() - 1;
				pause();
				restart();
			}
		}

		emit areaChanged();
		restartTimer();
	}
};
//! \endif

//!
//! Creates animation from QByteArray \a data.
Anim::Anim(const QByteArray &data)
{
	d = new Private(data);
}

//!
//! Creates shared copy of Anim \a anim.
Anim::Anim(const Anim &anim)
{
	d = anim.d;
	d->ref();
}

//!
//! Deletes animation.
Anim::~Anim()
{
	if ( d->deref() )
		delete d;
}

//!
//! Returns QPixmap of current frame.
const QPixmap &Anim::framePixmap() const
{
	return d->frames[d->frame]->impix->pixmap();
}

//!
//! Returns QImage of current frame.
const QImage &Anim::frameImage() const
{
	return d->frames[d->frame]->impix->image();
}

//!
//! Returns Impix of current frame.
const Impix &Anim::frameImpix() const
{
	return *d->frames[d->frame]->impix;
}

//!
//! Returns total number of frames in animation.
int Anim::numFrames() const
{
	return d->frames.count();
}

//!
//! Returns the number of current animation frame.
int Anim::framenumber() const
{
	return d->frame;
}

//!
//! Returns Impix of animation frame number \a n.
const Impix &Anim::frame(int n) const
{
	return *d->frames[n]->impix;
}

//!
//! Returns \c true if numFrames() == 0 and \c false otherwise.
bool Anim::isNull() const
{
	return !numFrames();
}

//!
//! Returns \c true when animation is paused.
bool Anim::paused() const
{
	return d->paused;
}

//!
//! Continues the animation playback.
void Anim::unpause()
{
	if ( !isNull() && paused() )
		d->unpause();
}

//!
//! Pauses the animation.
void Anim::pause()
{
	if ( !isNull() && !paused() )
		d->pause();
}

//!
//! Starts animation from the very beginning.
void Anim::restart()
{
	if ( !isNull() )
		d->restart();
}

/*!
	Connects internal signal with specified slot \a member of object \a receiver, which
	is emitted when animation changes its frame.

	\code
 class MyObject : public QObject {
 	Q_OBJECT
 	// ...
 public slots:
 	void animUpdated();
	// ...
	void foo (Anim *anim) {
		anim->connectUpdate( this, SLOT(animUpdated()) );
	}
 }
	\endcode

	\sa disconnectUpdate()
*/
void Anim::connectUpdate(QObject *receiver, const char *member)
{
	QObject::connect(d, SIGNAL(areaChanged()), receiver, member);
}

//!
//! Disconnects slot \a member, which was prevously connected with connectUpdate().
//! \sa connectUpdate()
void Anim::disconnectUpdate(QObject *receiver, const char *member)
{
	QObject::disconnect(d, SIGNAL(areaChanged()), receiver, member);
}

#include "anim.moc"
