/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "iconset.h"

void DisplayDlg::setIconset( const Iconset &i )
{
	is_display->setIconset(i);
	fancy->setText(QString("%1 v%2").arg(i.name()).arg(i.version()));
	fancy->setHelp(i.description());
}
