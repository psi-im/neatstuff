#include <qapplication.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdesktopwidget.h>

class ScreenTest : public QDialog
{
	Q_OBJECT
public:
	ScreenTest(QWidget *parent);
	~ScreenTest();

	QLabel *lbl;

private slots:
	void updateScreen();
};

ScreenTest::ScreenTest(QWidget *parent)
: QDialog(parent)
{
	QVBoxLayout *vbox = new QVBoxLayout(this);
	lbl = new QLabel(this);
	vbox->addWidget(lbl);

	QPushButton *pb = new QPushButton(this);
	vbox->addWidget(pb);
	pb->setText(tr("Get current screen number"));
	connect(pb, SIGNAL(clicked()), SLOT(updateScreen()));
	updateScreen();
}

ScreenTest::~ScreenTest()
{
}

void ScreenTest::updateScreen()
{
	QDesktopWidget *desk = qApp->desktop();
	QString txt;
	txt += QString("bool QDesktopWidget::isVirtualDesktop() = %1<br>").arg( desk->isVirtualDesktop() ? "true" : "false" );
	txt += QString("int QDesktopWidget::numScreens() = %1<br>").arg( desk->numScreens() );
	txt += QString("int QDesktopWidget::primaryScreen() = %1<br>").arg( desk->primaryScreen() );
	txt += QString("int QDesktopWidget::screenNumber(QWidget *widget = 0) = %1<br>").arg( desk->screenNumber(0) );
	txt += QString("int QDesktopWidget::screenNumber(QWidget *widget = this) = %1<br>").arg( desk->screenNumber(this) );
	lbl->setText(txt);
}

//-------------------------

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	ScreenTest *st = new ScreenTest(0);
	app.setMainWidget(st);
	st->show();

	return app.exec();
}

#include "main.moc"
