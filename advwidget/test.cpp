#include <qapplication.h>

#include "advwidget.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	AdvancedWidget *widget = new AdvancedWidget();
	app.setMainWidget(widget);

	widget->show();

	return app.exec();
}
