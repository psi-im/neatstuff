/****************************************************************************
** options.h - Options' handling class
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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <qvariant.h>

class QDomDocument;
class QDomElement;

class Options
{
public:
	Options();
	~Options();

	Options &operator=(const Options &);

	/* How to properly name properties:
	 *
	 *   1. ALL names should begin from leading slash. Examples:
	 *
	 *        * "/lookfeel/style"
	 *        * "/misc/blah"
	 *
	 *   2. Valid letters for use in names: [a-z], [A-Z], [0-9], '_', '-', '+'.
	 *      In general, all letters, that can be used as XML tag names, should be used freely.
	 *
	 *   3. It is possible to combine properties in groups, but there MUST NOT be property
	 *      with the name of group. Examples:
	 *
	 *      VALID:
	 *        * "/lookfeel/style" = "windowsxp"
	 *        * "/lookfeel/numFonts" = "4"
	 *        * "/lookfeel/blah" = "blah"
	 *
	 *      INVALID:
	 *        * "/lookfeel/style" = "windowsxp"
	 *        * "/lookfeel/numFonts" = "4"
	 *        * "/lookfeel" = "cool" <-- This is WRONG
	 *
	 *
	 * Example of imaginary property tree:
	 *
	 *   * "/test"
	 *   * "/lookfeel/style"
	 *   * "/lookfeel/numFonts"
	 *   * "/lookfeel/nested/something"
	 *   * "/lookfeel/nested/something_different"
	 *   * "/lookfeel/blah"
	 */

	QVariant property(const QString &) const;
	void setProperty(const QString &, const QVariant &);

	bool exists(const QString &);

	QDomElement toXml(QDomDocument *) const;
	bool fromXml(const QDomElement &);
	bool isEmpty() const;

private:
	class Private;
	Private *d;
};

#endif
