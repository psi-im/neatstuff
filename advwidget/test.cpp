#include <qapplication.h>

#include "advwidget.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

#include "ui_test.h"

class TestDialog : public AdvancedWidget<QWidget>
{
	Q_OBJECT
public:
	TestDialog();

private:
	TestWidget *d;

private slots:
	void setStickOn(bool b) { AdvancedWidget<QWidget>::setStickEnabled(b); }
	void setStickAt(int i) { AdvancedWidget<QWidget>::setStickAt(i); }
	void setStickWindowsOn(bool b) { AdvancedWidget<QWidget>::setStickToWindows(b); }
	void flashOn() { doFlash(true); }
	void flashOff() { doFlash(false); }
};

TestDialog::TestDialog()
{
	QVBoxLayout *vbox = new QVBoxLayout(this);
	d = new TestWidget(this);
	vbox->addWidget(d);

	setCaption(d->caption());

	connect(d->gb_stick, SIGNAL(toggled(bool)), SLOT(setStickOn(bool)));
	connect(d->sb_stick, SIGNAL(valueChanged(int)), SLOT(setStickAt(int)));
	connect(d->ck_stickWindows, SIGNAL(toggled(bool)), SLOT(setStickWindowsOn(bool)));

	connect(d->pb_flashOn, SIGNAL(clicked()), SLOT(flashOn()));
	connect(d->pb_flashOff, SIGNAL(clicked()), SLOT(flashOff()));

	resize(minimumSizeHint());
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	TestDialog *widget = new TestDialog();
	widget->show();
	app.setMainWidget(widget);

	AdvancedWidget<QWidget> *widget2 = new AdvancedWidget<QWidget>;
	widget2->show();

	return app.exec();
}

#include "test.moc"
