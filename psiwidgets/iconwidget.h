/****************************************************************************
** iconwidget.h - misc. widgets for displaying Iconsets
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

#ifndef ICONWIDGET_H
#define ICONWIDGET_H

#include <qlistbox.h>
#include <qpushbutton.h>

class QPaintEvent;

class Icon;
class Iconset;

class IconsetSelectItem;

class IconsetSelect : public QListBox
{
	Q_OBJECT
public:
	IconsetSelect(QWidget *parent = 0, const char *name = 0);
	~IconsetSelect();

	void insert(const Iconset &); // iconsets must be inserted in following order: most prioritent first

	const Iconset *iconset() const;

public slots:
	void moveItemUp();
	void moveItemDown();

private:
	class Private;
	Private *d;

	void paintCell(QPainter *p, int row, int col);

	friend class IconsetSelectItem;
};

class IconWidgetItem : public QObject, public QListBoxItem
{
	Q_OBJECT
public:
	IconWidgetItem(QListBox *parent = 0, QListBoxItem *after = 0)
	: QListBoxItem(parent, after) {}

	const Iconset *iconset() const { return 0; }
};

class IconsetDisplayItem;

class IconsetDisplay : public QListBox
{
	Q_OBJECT
public:
	IconsetDisplay(QWidget *parent = 0, const char *name = 0);
	~IconsetDisplay();

	void setIconset(const Iconset &);
private:
	class Private;
	Private *d;

	void paintCell(QPainter *p, int row, int col);
	
	friend class IconsetDisplayItem;
};

class IconButton : public QPushButton
{
	Q_OBJECT
	Q_PROPERTY( QString iconName READ iconName WRITE setIcon )
	Q_PROPERTY( bool textVisible READ textVisible WRITE setTextVisible )
	
	Q_OVERRIDE( QPixmap pixmap DESIGNABLE false SCRIPTABLE false )
	Q_OVERRIDE( QIconSet iconSet DESIGNABLE false SCRIPTABLE false )

public:
	IconButton(QWidget *parent = 0, const char *name = 0);
	~IconButton();

public slots:
	void setIcon(const Icon *);
	void setIcon(const QString &);
	const QString &iconName() const;

	void setText(const QString &);

	bool textVisible() const;
	void setTextVisible(bool);

public:
	class Private;
private:
	Private *d;

	void drawButtonLabel(QPainter *p);
};

#endif
