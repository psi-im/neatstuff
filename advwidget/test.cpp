#include <qapplication.h>

#include "advwidget.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	AdvancedWidget *widget = new AdvancedWidget();
	widget->show();
	app.setMainWidget(widget);

	AdvancedWidget *widget2 = new AdvancedWidget();
	widget2->show();

	return app.exec();
}
