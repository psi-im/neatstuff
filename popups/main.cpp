#include <qapplication.h>
#include <qdir.h>

#include "fancypopup.h"
#include "psipng.h"
#include "iconset.h"
#include "dialog.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	/*SelectDlg *sd = new SelectDlg();
	app.setMainWidget(sd);
	sd->show();*/

	initPsiPngIO();

	QPtrList<Iconset> isList;
	isList.setAutoDelete(true);

	QDir dir ("../test/emoticons");
	QStringList list = dir.entryList("*");
	QStringList::Iterator it = list.begin();
	for ( ; it != list.end(); ++it) {
		if ( *it == "." || *it == ".." )
			continue;

		Iconset *is = new Iconset;
		is->load ("../test/emoticons/" + *it);

		is->addToFactory();
		isList.append ( is );
	}

	/*FancyPopup *pop = new FancyPopup();
	app.setMainWidget(pop);
	pop->show();*/
	PopupDlg *dlg = new PopupDlg();
	app.setMainWidget(dlg);
	dlg->show();

	int ret = app.exec();
	return ret;
}
