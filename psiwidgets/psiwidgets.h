/****************************************************************************
** psiwidgets.h - plugin for loading Psi's custom widgets into Qt Designer
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

#ifndef PSIWIDGETSPLUGIN_H
#define PSIWIDGETSPLUGIN_H

#include <qwidgetplugin.h>

class QT_WIDGET_PLUGIN_EXPORT PsiWidgetsPlugin : public QWidgetPlugin
{
public:
	PsiWidgetsPlugin();

	QStringList keys() const;
	QWidget *create(const QString &classname, QWidget *parent = 0, const char *name = 0 );
	QString group(const QString &) const;
	QIconSet iconSet(const QString &) const;
	QString includeFile(const QString &) const;
	QString toolTip(const QString &) const;
	QString whatsThis(const QString &) const;
	bool isContainer(const QString &) const;
};

#endif
