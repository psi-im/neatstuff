#ifndef PSITEXTVIEW
#define PSITEXTVIEW

#include <qtextedit.h>
#include <qlabel.h>

class URLLabel : public QLabel
{
	Q_OBJECT

	Q_PROPERTY( QString url READ url WRITE setUrl )
	Q_PROPERTY( QString title READ title WRITE setTitle )

	Q_OVERRIDE( QString text DESIGNABLE false SCRIPTABLE false )
	//Q_OVERRIDE( TextFormat DESIGNABLE false SCRIPTABLE false )

public:
	URLLabel(QWidget *parent = 0, const char *name = 0);

	const QString &url() const;
	void setUrl(const QString &);

	const QString &title() const;
	void setTitle(const QString &);

protected:
	virtual void mouseReleaseEvent (QMouseEvent *);
	virtual void enterEvent (QEvent *);
	virtual void leaveEvent (QEvent *);

	void updateText();

public:
	class Private;
private:
	Private *d;
};

class PsiTextView : public QTextEdit
{
	Q_OBJECT

	Q_OVERRIDE( int undoDepth DESIGNABLE false SCRIPTABLE false )
	Q_OVERRIDE( bool overwriteMode DESIGNABLE false SCRIPTABLE false )
	Q_OVERRIDE( bool modified SCRIPTABLE false)
	Q_OVERRIDE( bool readOnly DESIGNABLE false SCRIPTABLE false )
	Q_OVERRIDE( bool undoRedoEnabled DESIGNABLE false SCRIPTABLE false )

public:
	PsiTextView(QWidget *parent = 0, const char *name = 0);

	class Private;
private:
	QPopupMenu *createPopupMenu(const QPoint &pos);

	bool linksEnabled() const { return TRUE; }
	void emitHighlighted(const QString &s);
	void emitLinkClicked(const QString &s);

	Private *d;
};

#endif
