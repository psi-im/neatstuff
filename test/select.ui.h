/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "iconset.h"
#include "display.h"
#include "iconselect.h"

void SelectDlg::displayAll()
{
	const Iconset *is = is_select->iconset();
	if ( is ) {
		DisplayDlg *dd = new DisplayDlg(this, "IconsetDisplay", true);
		dd->setIconset(*is);
		dd->exec();
		delete dd;
	}
}

void SelectDlg::yahoo()
{
	const Iconset *is = is_select->iconset();
	if ( is ) {
		if (iconNum >= (int)is->count())
			iconNum = 0;

		QDictIterator<Icon> it = is->iterator();
		for (int counter = 0; it.current(); ++it, counter++) {
			if (counter == iconNum) {
				ib_test->setIcon( *it );
				break;
			}
		}

		iconNum++;
	}
}

void SelectDlg::selectIcon()
{
}

void SelectDlg::selectionChanged()
{
	iconNum = 0;

	const Iconset *is = is_select->iconset();
	if ( is )
		iconSelectPopup->setIconset( Iconset(*is) );
}


void SelectDlg::init()
{
	iconNum = 0;
	iconSelectPopup = new IconSelectPopup(this);
	ib_showPopup->setPopup( iconSelectPopup );
}
