#include <qapplication.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include "qgtk.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyle( new QGtk );

	QWidget pw;
	app.setMainWidget(&pw);

	QVBoxLayout *vbox = new QVBoxLayout(&pw, 10, 10);

	QPushButton *pb = new QPushButton("Blah", &pw);
	vbox->addWidget(pb);

	QCheckBox *cb = new QCheckBox("Blah", &pw);
	vbox->addWidget(cb);

	pw.show();

	return app.exec();
}
