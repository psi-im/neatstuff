#ifndef ADVWIDGET_H
#define ADVWIDGET_H

#include <qwidget.h>

class GAdvancedWidget : public QObject
{
	Q_OBJECT
public:
	GAdvancedWidget(QWidget *parent, const char *name = 0);
	~GAdvancedWidget();

	static bool stickEnabled();
	static void setStickEnabled(bool val);

	static int stickAt();
	static void setStickAt(int val);

	static bool stickToWindows();
	static void setStickToWindows(bool val);

	void doFlash(bool on);

#ifdef Q_OS_WIN
	bool winEvent(MSG *msg);
#endif

	void setCaption(const QString &);

	void windowActivationChange(bool oldstate);

public:
	class Private;
private:
	Private *d;
};

template <class BaseClass>
class AdvancedWidget : public BaseClass
{
private:
	GAdvancedWidget *gAdvWidget;

public:
	AdvancedWidget( QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0 )
		: BaseClass( parent, name, f )
	{
		gAdvWidget = new GAdvancedWidget( this );
	}

	static bool stickEnabled() { return GAdvancedWidget::stickEnabled(); }
	static void setStickEnabled(bool val) { GAdvancedWidget::setStickEnabled( val ); }

	static int stickAt() { return GAdvancedWidget::stickAt(); }
	static void setStickAt(int val) { GAdvancedWidget::setStickAt( val ); }

	static bool stickToWindows() { return GAdvancedWidget::stickToWindows(); }
	static void setStickToWindows(bool val) { GAdvancedWidget::setStickToWindows( val ); }

	void doFlash(bool on)
	{
		gAdvWidget->doFlash( on );
	}

#ifdef Q_OS_WIN
	bool winEvent(MSG *msg)
	{
		return gAdvWidget->winEvent( msg );
	}
#endif

	void setCaption(const QString &c)
	{
		gAdvWidget->setCaption( c );
		BaseClass::setCaption( c );
	}

protected:
	void windowActivationChange(bool oldstate)
	{
		gAdvWidget->windowActivationChange( oldstate );
		BaseClass::windowActivationChange( oldstate );
	}
};

#endif
