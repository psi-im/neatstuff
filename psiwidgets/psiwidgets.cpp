/****************************************************************************
** psiwidgets.cpp - plugin for loading Psi's custom widgets into Qt Designer
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

#include "psiwidgets.h"

#include "fancylabel.h"
#include "busywidget.h"
#include "iconwidget.h"
#include "psitextview.h"

static const char* const psiwidget_data[] = { 
"16 16 5 1",
". c None",
"# c #000000",
"c c #57acda",
"b c #72bde6",
"a c #cde9f8",
".###..####..###.",
"#aaa#.#aa#.#aaa#",
"#abbb##ac##abcc#",
".##ab##ac##ac##.",
"..#abc#ac#abc#..",
"..#abc#ac#abc#..",
"..#abc#ac#abc#..",
"..#abc#ac#abc#..",
"..#abbbbbbbbc#..",
"...#abbbbbcc#...",
"....##cccc##....",
"......#ac#......",
"......#ac#......",
"......#ac#......",
"......#ac#......",
"......####......"};

#define iconlabel_data psiwidget_data
#define fancylabel_data psiwidget_data
#define busywidget_data psiwidget_data
#define iconsetselect_data psiwidget_data
#define iconsetdisplay_data psiwidget_data
#define iconbutton_data psiwidget_data
#define psitextview_data psiwidget_data
#define urllabel_data psiwidget_data

PsiWidgetsPlugin::PsiWidgetsPlugin()
{
}

QStringList PsiWidgetsPlugin::keys() const
{
	QStringList list;
	list << "IconLabel";
	list << "FancyLabel";
	list << "BusyWidget";
	list << "IconsetSelect";
	list << "IconsetDisplay";
	list << "IconButton";
	list << "PsiTextView";
	list << "URLLabel";
	return list;
}

QWidget *PsiWidgetsPlugin::create(const QString &key, QWidget *parent, const char *name)
{
	if ( key == "IconLabel" )
		return new IconLabel( parent, name );
	if ( key == "FancyLabel" )
		return new FancyLabel( parent, name );
	if ( key == "BusyWidget" )
		return new BusyWidget( parent, name );
	if ( key == "IconsetSelect" )
		return new IconsetSelect( parent, name );
	if ( key == "IconsetDisplay" )
		return new IconsetDisplay( parent, name );
	if ( key == "IconButton" )
		return new IconButton( parent, name );
	if ( key == "PsiTextView" )
		return new PsiTextView( parent, name );
	if ( key == "URLLabel" )
		return new URLLabel( parent, name );
	return 0;
}

QString PsiWidgetsPlugin::includeFile(const QString &feature) const
{
	if ( feature == "IconLabel" || feature == "FancyLabel" )
		return "fancylabel.h";
	if ( feature == "BusyWidget" )
		return "busywidget.h";
	if ( feature == "IconsetSelect" || feature == "IconsetDisplay" || feature == "IconButton" )
		return "iconwidget.h";
	if ( feature == "PsiTextView" || feature == "URLLabel" )
		return "psitextview.h";
	return QString::null;
}

QString PsiWidgetsPlugin::group(const QString &feature) const
{
	if ( feature == "IconLabel" || feature == "FancyLabel" )
		return "Display";
	if ( feature == "BusyWidget" )
		return "Display";
	if ( feature == "IconsetSelect" || feature == "IconsetDisplay" )
		return "Views";
	if ( feature == "IconButton" )
		return "Buttons";
	if ( feature == "PsiTextView" || feature == "URLLabel" )
		return "Display";
	return QString::null;
}

QIconSet PsiWidgetsPlugin::iconSet(const QString &key) const
{
	if ( key == "IconLabel" )
		return QIconSet( QPixmap( (const char **)iconlabel_data ) );
	if ( key == "FancyLabel" )
		return QIconSet( QPixmap( (const char **)fancylabel_data ) );
	if ( key == "BusyWidget" )
		return QIconSet( QPixmap( (const char **)busywidget_data ) );
	if ( key == "IconsetSelect" )
		return QIconSet( QPixmap( (const char **)iconsetselect_data ) );
	if ( key == "IconsetDisplay" )
		return QIconSet( QPixmap( (const char **)iconsetdisplay_data ) );
	if ( key == "IconButton" )
		return QIconSet( QPixmap( (const char **)iconbutton_data ) );
	if ( key == "PsiTextView" )
		return QIconSet( QPixmap( (const char **)psitextview_data ) );
	if ( key == "URLLabel" )
		return QIconSet( QPixmap( (const char **)psitextview_data ) );
	return QIconSet();
}

QString PsiWidgetsPlugin::toolTip(const QString &feature) const
{
	if ( feature == "IconLabel" )
		return "Icon Label";
	if ( feature == "FancyLabel" )
		return "Fancy Label";
	if ( feature == "BusyWidget" )
		return "Busy Widget";
	if ( feature == "IconsetSelect" )
		return "Iconset Select";
	if ( feature == "IconsetDisplay" )
		return "Iconset Display";
	if ( feature == "IconButton" )
		return "Icon Button";
	if ( feature == "PsiTextView" )
		return "Psi's Text View";
	if ( feature == "URLLabel" )
		return "URL Label";
	return QString::null;
}

QString PsiWidgetsPlugin::whatsThis(const QString &feature) const
{
	if ( feature == "IconLabel" )
		return "Label that can contain animated Icon.";
	if ( feature == "FancyLabel" )
		return "Just a Fancy Label. Use it for decoration of dialogs. ;-)";
	if ( feature == "BusyWidget" )
		return "Widget for indicating that program is doing something.";
	if ( feature == "IconsetSelect" )
		return "Widget for Iconset selection.";
	if ( feature == "IconsetDisplay" )
		return "Displays all icons in Iconset.";
	if ( feature == "IconButton" )
		return "PushButton that can contain animated Icon.";
	if ( feature == "PsiTextView" )
		return "Widget for displaying rich-text data, with inline Icons.";
	if ( feature == "URLLabel" )
		return "Widget for displaying clickable URLs.";
	return QString::null;
}

bool PsiWidgetsPlugin::isContainer(const QString &key) const
{
	/*if ( key == "FancyLabel" )
		return FALSE;
	if ( key == "BusyWidget" )
		return FALSE;
	if ( key == "IconsetSelect" )
		return FALSE;
	if ( key == "IconsetDisplay" )
		return FALSE;
	if ( key == "IconButton" )
		return FALSE;*/
	return FALSE;
}
    
Q_EXPORT_PLUGIN( PsiWidgetsPlugin )
