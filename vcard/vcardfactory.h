/****************************************************************************
** vcardfactory.h - class for caching vCards
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

#ifndef VCARDFACTORY_H
#define VCARDFACTORY_H

#include "vcard.h"

#include "jid.h"
//class Jid;
class Task;
class QObject;

using namespace Jabber;

class VCardFactory
{
public:
	static const VCard *vcard(const Jid &);
	static void getVCard(const Jid &, const Task *rootTask, const QObject *, const char *slot);
};

#endif
