#include <qapplication.h>

#include "iconwidget.h"
#include "iconselect.h"
#include "psipng.h"
#include "iconset.h"
#include "psitextview.h"

#include "select.h"

//#define SELECT_TEST
//#define DISPLAY_TEST
//#define BUTTON_TEST
//#define TEXTVIEW_TEST
//#define URL_TEST
#define DIALOG_TEST

#include <qdir.h>
#include <qptrlist.h>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	initPsiPngIO();

	QPtrList<Iconset> isList;
	isList.setAutoDelete(true);

	QDir dir ("emoticons");
	QStringList list = dir.entryList("*");
	QStringList::Iterator it = list.begin();
	for ( ; it != list.end(); ++it) {
		if ( *it == "." || *it == ".." )
			continue;

		Iconset *is = new Iconset;
		is->load ("emoticons/" + *it);

		is->addToFactory();
		isList.append ( is );
	}

	/*
	qWarning("Registered icons:");
	QStringList list = IconsetFactory::icons();
	QStringList::Iterator it;
	for (it = list.begin(); it != list.end(); ++it)
		qWarning("\t%s", (*it).latin1());
	*/

#ifdef SELECT_TEST
	IconsetSelect *iss = new IconsetSelect();
	app.setMainWidget(iss);

	for (int i = 0; i < num; i++)
		iss->insert( is[i] );
	iss->show();
#endif

#ifdef DISPLAY_TEST
	IconsetDisplay *isd = new IconsetDisplay();
	app.setMainWidget(isd);

	isd->setIconset(is[1]);
	isd->show();
#endif

#ifdef BUTTON_TEST
	IconButton *ib = new IconButton;
	app.setMainWidget(ib);

	ib->setIcon( &is[5].icon("message/chat") );
	ib->setText("Blah");
	ib->show();
#endif

#ifdef TEXTVIEW_TEST
	PsiTextView *ptv = new PsiTextView;
	app.setMainWidget(ptv);

	QString text =	"<a href=\"http://psi.affinix.com\">Psi Homepage</a> Yo-Yo-Yo! <icon src=\"psi\"><br>"
			"Lorem ipsum blah blah blah blah blah blah <icon name=\"test/fire\"> blah blah blah blah...";

	ptv->setText( text );
	ptv->show();
#endif

#ifdef URL_TEST
	URLLabel *url = new URLLabel;
	app.setMainWidget(url);

	url->setUrl("http://blah.com");
	url->setTitle("test");

	url->show();
#endif

#ifdef DIALOG_TEST
	SelectDlg *sd = new SelectDlg();
	app.setMainWidget(sd);

	QPtrListIterator<Iconset> it2 ( isList );
	for ( ; it2.current(); ++it2)
		sd->is_select->insert( **it2 );

	sd->is_select->setCurrentItem(0);
	sd->show();
#endif

	int ret = app.exec();

#ifdef DIALOG_TEST
	delete sd;
	isList.clear();
#endif

	return ret;
}
