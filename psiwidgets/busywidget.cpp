/*
 * busywidget.cpp - cool animating widget
 * Copyright (C) 2001, 2002  Justin Karneges
 *                           Hideaki Omuro
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include"busywidget.h"

#include<qtimer.h>
#include<qpainter.h>
#include<math.h>

/////////////////////////////////////////////////////////////////////////////
// common defines
//
#define FPS          20 // frequency of update

#define SPINRATE     24 // 1024ths of a revolution per frame
#define SPINOFFSET    4 // frames each panel is offset by

#define COLORSTOPPED  0xD3D0D3 // color when stopped
#define COLORSPINNING 0xFF00FF // color when spinning
#define COLORSHADOW   0x000000 // color of shadow

/////////////////////////////////////////////////////////////////////////////
// derived defines
//
#define MSECSPERFRAME	(1000 / FPS)

/////////////////////////////////////////////////////////////////////////////
// declared later
//
extern char psigraphic[];

// panel class
class CPanel
{
public:
	int angle;
	int height;
	bool spinning;
	int alpha;

	CPanel(int height = 1);
	int GetModHeight();
	int GetShade();
	void Spin(int n);

	void SetAngle(int _angle)	{ angle = _angle % 1024; }
	void SetHeight(int _height)	{ height = _height; }
	int GetAngle()				{ return angle % 1024; }
	int GetHeight()				{ return height; }
	int GetModOffset()			{ return (height - GetModHeight()) / 2; }
};

// color class
class CColor
{
public:
	int m_clr;

	CColor(int _r, int _g, int _b)
	{
		SetColor(_r, _g, _b);
	}
	CColor(int _clr)
	{
		SetColor(_clr);
	}
	inline void SetColor(int _r, int _g, int _b)
	{
		SetColor((_r << 16) + (_g << 8) + _b);
	}
	inline void SetColor(int _clr)
	{
		m_clr = _clr;
	}
	inline short GetR()
	{
		return m_clr >> 16;
	}
	inline short GetG()
	{
		return (m_clr >> 8) & 255;
	}
	inline short GetB()
	{
		return m_clr & 255;
	}
	CColor Alpha(CColor clr, int alpha = 256);
};

class BusyWidget::Private
{
private:
	BusyWidget *busy;
	
public:
	Private(BusyWidget *b) 
	{
		t = 0;
		busy = b;
	}
	
	bool isActive;
	int frame;
	int at;
	QPixmap pix;
	QTimer *t;

	// data
	CPanel panel[5];
	int  pcountdown;
	int  ocountdown;

	void render()
	{
		QPainter p(busy);
		p.drawPixmap(0,0, pix);
	}
	
	void renderPixmap()
	{
		pix.resize(busy->width(), busy->height());
		pix.fill(QColor("#406080"));

		QPainter p(&pix);

		int i, j, k, l;
		int row;

		for(i = 0; i < 5; i++)
		{
			int o = panel[i].GetModOffset();

			CColor c1(COLORSPINNING), c2(COLORSTOPPED), c3(COLORSHADOW);
			CColor t = c1.Alpha(c2, panel[i].alpha * 8);

			CColor a = t.Alpha(c3, panel[i].GetShade());

			l = panel[i].GetModHeight();

			double radangle = (double) 3.1415926f * (double) panel[i].GetAngle() / (double) 512;
			int step = (int)((double)1024 / cos(radangle));
			step = step < 0 ? -step : step;

			int n = (int)((double)1024 * cos(radangle) * 17 / 2);
			n = n < 0 ? -n : n;

			row = 8192 - step * n / 1024;

			QColor clr(a.GetR(), a.GetG(), a.GetB());

			for(j = 0; j < l; j++)
			{
				int m = row / 1024 + 1;
				for(k = 0; k < 16; k++)
				{
					p.setPen(psigraphic[i * 304 + m * 16 + k] ? Qt::black : clr);
					p.drawPoint(i * 16 + k + 1, o + j + 1);
				}
				row += step;
			}
		}

		p.setPen(Qt::black);
		p.drawRect(0, 0, busy->width(), busy->height());
	}
};

/////////////////////////////////////////////////////////////////////////////
// code
//
BusyWidget::BusyWidget(QWidget *parent, const char *name)
:QWidget(parent, name)
{
	d = new Private(this);

	d->isActive = FALSE;
	d->frame = 0;
	d->at = 0;

	d->pcountdown = 0;
	d->ocountdown = 0;

	setFixedWidth(82);
	setFixedHeight(19);
	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

	int i;
	for(i = 0; i < 5; i++)
		d->panel[i].SetHeight(17);

	d->renderPixmap();
}

BusyWidget::~BusyWidget()
{
	delete d;
}

bool BusyWidget::isActive() const
{
	return d->isActive;
}

void BusyWidget::setActive(bool a)
{
	if ( a )
		start();
	else
		stop();
}

void BusyWidget::start()
{
	if(d->isActive)
		return;

	d->isActive = TRUE;

	if(!d->pcountdown)
		d->pcountdown = SPINOFFSET * 4 + 1;

	if(!d->t)
	{
		d->t = new QTimer(this);
		connect(d->t, SIGNAL(timeout()), SLOT(animate()));
		animate();
		d->t->start(MSECSPERFRAME);
	}
}

void BusyWidget::stop()
{
	if(!d->isActive)
		return;

	d->isActive = FALSE;

	if(!d->ocountdown)
		d->ocountdown = SPINOFFSET * 4 + 1;
}

void BusyWidget::animate()
{
	int i;
	for(i = 0; i < 5; i++)
		d->panel[i].Spin(SPINRATE);

	if(d->pcountdown)
		if(!(--d->pcountdown % SPINOFFSET))
			d->panel[d->pcountdown / SPINOFFSET].spinning = true;

	if(d->ocountdown)
		if(!(--d->ocountdown % SPINOFFSET))
			d->panel[d->ocountdown / SPINOFFSET].spinning = false;

	if(!d->isActive)
	{
		bool isValid = false;
		for(i = 0; i < 5; i++)
			if(d->panel[i].spinning || d->panel[i].GetAngle() != 0 || d->panel[i].alpha != 0)
				isValid = true;

		if(!isValid)
		{
			delete d->t;
			d->t = 0;
		}
	}

	d->renderPixmap();
	d->render();
}

void BusyWidget::paintEvent(QPaintEvent *)
{
	d->render();
}

void BusyWidget::resizeEvent(QResizeEvent *)
{
	d->renderPixmap();
}

/////////////////////////////////////////////////////////////////////////
// stuff beyond here for animating rotating psi panels
//

// color stuff
CColor CColor::Alpha(CColor clr, int alpha)
{
	int ialpha = 256 - alpha;

	int r, g, b;
	r = (alpha * GetR() + ialpha * clr.GetR()) / 256;
	g = (alpha * GetG() + ialpha * clr.GetG()) / 256;
	b = (alpha * GetB() + ialpha * clr.GetB()) / 256;

	return CColor(r, g, b);
}

// panel stuff
CPanel::CPanel(int _height)
{
	height = _height;
	spinning = false;
	angle = 0;
	alpha = 0;
}

int CPanel::GetModHeight()
{
	int l = GetAngle();
	if(l > 512)
		l = 1024 - l;
	double radangle = (double) 3.1415926f * (double) l / (double) 512;
	int tmp = (int)(cos(radangle)* (double) height);
	return tmp < 0 ? -tmp : tmp;
}

int CPanel::GetShade()
{
	int l = GetAngle() + 128;
	if(GetAngle() >= 256 && GetAngle() < 768)
		l += 512;
	if(l >= 1024)
		l %= 1024;
	if(l == 0)
		l += 1024;
	double radangle = (double) 3.1415926f * (double) l / (double) 512;
	return 128 + (int)(cos(radangle)* (double) 128);
}

void CPanel::Spin(int n)
{
	int i = angle + n;
	if(!spinning)
	{
		if(i >= 1024)
			SetAngle(0);
		if(angle < 512 && i >= 512)
			SetAngle(0);
		if(angle)
			SetAngle(i);
	}
	else
		SetAngle(i);
	if(spinning)
	{
		if(alpha < 32)
			alpha+=2;
	}
	else
	{
		if(alpha)
			alpha-=2;
	}
}

char psigraphic[304*5] =
{
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

 	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
	 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
