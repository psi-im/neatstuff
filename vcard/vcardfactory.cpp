/****************************************************************************
** vcardfactory.cpp - class for caching vCards
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

#include "vcardfactory.h"

#ifdef TASKS
#include "tasks.h"
#endif
#include "jid.h"

#include "common.h" // jidEncode

#include <qobject.h>
#include <qapplication.h>

#include <qdict.h>
#include <qmap.h>

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

//----------------------------------------------------------------------------
// VCardFactory
//----------------------------------------------------------------------------

class VCardFactoryPrivate : public QObject
{
	Q_OBJECT

private:
	static QDict<VCard> *vcardDict;

public slots:
	void taskFinished();

public:
	VCardFactoryPrivate();
	~VCardFactoryPrivate();

	static QDict<VCard> *vcards();
};

QDict<VCard> *VCardFactoryPrivate::vcardDict = 0;

VCardFactoryPrivate::VCardFactoryPrivate()
: QObject(qApp, "VCardFactoryPrivate")
{
	vcardDict = new QDict<VCard>;
	vcardDict->setAutoDelete(true);
}

VCardFactoryPrivate::~VCardFactoryPrivate()
{
	delete vcardDict;
}

QDict<VCard> *VCardFactoryPrivate::vcards()
{
	return vcardDict;
}

void VCardFactoryPrivate::taskFinished()
{
	JT_VCard *task = (JT_VCard *)sender();
	if ( task->success() ) {
		Jid j = task->jid();
		if ( vcardDict->find( j.userHost() ) )
			vcardDict->remove( j.userHost() );

		VCard *vcard = new VCard;
		*vcard = task->vcard();
		vcards.insert (j.userHost(), vcard);

		// save vCard to disk
		QFile file ( "vcard/" + jidEncode(j.userHost()).lower() + ".xml" );
		file.open ( IO_WriteOnly );
		QTextStream out ( &file2 );
		out.setEncoding (QTextStream::UnicodeUTF8);
		QDomDocument doc;
		doc.appendChild( vcard.toXml ( &doc ) );
		out << doc.toString(4);
	}
}

static VCardFactoryPrivate *vcardFactoryPrivate = 0;

const VCard *VCardFactory::vcard(const Jid &j)
{
	if ( !vcardFactoryPrivate )
		vcardFactoryPrivate = new VCardFactoryPrivate;

	// first, try to get vCard from runtime cache
	QDict<VCard> *vcards = VCardFactoryPrivate::vcards();
	if ( vcards->find( j.userHost() ) )
		return vcards->find( j.userHost() );

	qDebug("%s", j.userHost().latin1());

	// then try to load from cache on disk
	QFile file ( "vcard/" + jidEncode(j.userHost()).lower() + ".xml" );
	file.open (IO_ReadOnly);
	QDomDocument doc;
	VCard *vcard = new VCard;
	if ( doc.setContent(&file, false) ) {
		vcard->fromXml( doc.documentElement() );
		vcards->insert (j.userHost(), vcard);

		return vcard;
	}
	else {
		delete vcard;
	}

	return 0;
}

void VCardFactory::getVCard(const Jid &jid, const Task *rootTask, const QObject *obj, const char *slot)
{
	if ( !vcardFactoryPrivate )
		vcardFactoryPrivate = new VCardFactoryPrivate;

	JT_VCard *task = new JT_VCard( rootTask );
	task->connect(task, SIGNAL(finished()), vcardFactoryPrivate, SLOT(taskFinished()));
	task->connect(task, SIGNAL(finished()), obj, slot);
	task->get(jid);
	task->go(true);
}

#include "vcardfactory.moc"
