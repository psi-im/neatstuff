/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "popuplist.h"

void PopupDlg::addPopup()
{
	FancyPopup *last = 0;
	if ( popups->count() )
		last = popups->last();
	
	FancyPopup *pop = new FancyPopup(last, le_title->text(), le_titleIcon->text(),
					 le_statusIcon->text(), le_name->text(), le_jid->text(), te_status->text());
	popups->prepend( pop );
	
	pop->show();
}


void PopupDlg::init()
{
	popups = new PopupList;
}


void PopupDlg::destroy()
{
	delete popups;
}
