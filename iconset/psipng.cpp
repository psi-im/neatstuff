/****************************************************************************
** psipng.cpp - QImageFormat for loading Psi .png animations
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

#include "psipng.h"

#include <qasyncimageio.h>

class PsiPNGFormat : public QImageFormat {
private:
	int w, h;
	QImage image;
	int numFrames;
	int frame, frameW;
	int frameBytes;
	
	int maxLen, curLen;
	
	static const int framePeriod = 100;
	
public:
	PsiPNGFormat()
	{
	}
	
	~PsiPNGFormat()
	{
	}

	int decode(QImage &img, QImageConsumer *consumer, const uchar *buffer, int length)
	{
		if ( !length )
			return 0;
		
		if ( image.isNull() ) { // cache our image for speedup
			if ( image.loadFromData( buffer, length ) ) {
				w = image.width();
				h = image.height();
				
				numFrames = w / h;
				frameBytes = length / numFrames;
				frameW = w / numFrames;
				frame = 0;
				
				consumer->setLooping( numFrames > 1 ? 0 : 1 );
				consumer->setFramePeriod(framePeriod);
				consumer->setSize(frameW, h);
				
				maxLen = length;
				curLen = 0;
			}
			else {
				if ( !image.isNull() )
					qWarning("PsiPNGFormat::decode: WARNING: QImage is not loaded and is NOT null!!!");
				return 0;
			}
		}
				
		if ( frame < numFrames ) {
			img = image.copy(frame++ * frameW, 0, frameW, h);
			consumer->frameDone(QPoint(0, 0), QRect(0, 0, frameW, h));
			
			if ( frame >= numFrames ) {
				consumer->end();
				frameBytes = maxLen - curLen;
			}
			else
				consumer->setFramePeriod(framePeriod);
		}
		
		curLen += frameBytes;
		return frameBytes;
	}
};

class PsiPNGFormatType : public QImageFormatType
{
	QImageFormat *decoderFor(const uchar *buffer, int length)
	{
		if (length < 8) 
			return 0;
			
		if (buffer[0]==137
		 && buffer[1]=='P'
		 && buffer[2]=='N'
		 && buffer[3]=='G'
		 && buffer[4]==13
		 && buffer[5]==10
		 && buffer[6]==26
		 && buffer[7]==10)
			return new PsiPNGFormat;
			
		return 0;
	}
	
	const char *formatName() const
	{
		return "PsiPNG";
	}
};

PsiPNGFormatType *globalPsiPngFormatTypeObject = 0;

void cleanupPsiPngIO()
{
	if ( globalPsiPngFormatTypeObject ) {
		delete globalPsiPngFormatTypeObject;
		globalPsiPngFormatTypeObject = 0;
	}
}

void initPsiPngIO()
{
	static bool done = FALSE;
	if ( !done ) {
		done = TRUE;
		globalPsiPngFormatTypeObject = new PsiPNGFormatType;
		qAddPostRoutine( cleanupPsiPngIO );
	}
}
