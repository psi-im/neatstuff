#include "xdata_widget.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <qobjectlist.h>
#include <qtooltip.h>
#include <qtextedit.h>

//----------------------------------------------------------------------------
// XDataField
//----------------------------------------------------------------------------
class XDataField
{
public:
	XDataField(XData::Field f)
	{
		_field = f;
	}
	virtual ~XDataField()
	{
	}

	virtual XData::Field field() const
	{
		return _field;
	}

	QString labelText() const
	{
		QString text = _field.label();
		if ( text.isEmpty() )
			text = _field.var();
		return text + ": ";
	}

	QString reqText() const
	{
		QString req;
		if ( _field.required() )
			req = "*";
		if ( !_field.desc().isEmpty() ) {
			if ( !req.isEmpty() )
				req += " ";
			req += "(?)";
		}
		return req;
	}

	virtual bool isValid() const
	{
		return field().isValid();
	}

private:
	XData::Field _field;
};

////////////////////////////////////////

class XDataField_Hidden : public XDataField
{
public:
	XDataField_Hidden(XData::Field f)
	: XDataField(f)
	{
	}
};

////////////////////////////////////////

class XDataField_Boolean : public XDataField
{
public:
	XDataField_Boolean(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField(f)
	{
		bool checked = false;
		if ( f.value().count() ) {
			QString s = f.value().first();
			if ( s == "1" || s == "true" || s == "yes" )
				checked = true;
		}

		QLabel *label = new QLabel(labelText(), parent);
		grid->addWidget(label, row, 0);

		check = new QCheckBox(parent);
		check->setChecked(checked);
		grid->addWidget(check, row, 1);

		QLabel *req = new QLabel(reqText(), parent);
		grid->addWidget(req, row, 2);

		if ( !f.desc().isEmpty() ) {
			QToolTip::add(label, f.desc());
			QToolTip::add(check, f.desc());
			QToolTip::add(req, f.desc());
		}
	}

	XData::Field field() const
	{
		XData::Field f = XDataField::field();
		QStringList val;
		val << QString( check->isChecked() ? "true" : "false" );
		f.setValue(val);
		return f;
	}

private:
	QCheckBox *check;
};

////////////////////////////////////////

class XDataField_Fixed : public XDataField
{
public:
	XDataField_Fixed(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField(f)
	{
		QString text;
		QStringList val = f.value();
		QStringList::Iterator it = val.begin();
		for ( ; it != val.end(); ++it) {
			if ( !text.isEmpty() )
				text += "<br>";
			text += *it;
		}

		QLabel *fixed = new QLabel("<qt>" + text + "</qt>", parent);
		grid->addMultiCellWidget(fixed, row, row, 0, 2);

		if ( !f.desc().isEmpty() ) {
			QToolTip::add(fixed, f.desc());
		}
	}
};

////////////////////////////////////////

class XDataField_TextSingle : public XDataField
{
public:
	XDataField_TextSingle(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField(f)
	{
		QString text;
		if ( f.value().count() )
			text = f.value().first();

		QLabel *label = new QLabel(labelText(), parent);
		grid->addWidget(label, row, 0);

		edit = new QLineEdit(parent);
		edit->setText(text);
		grid->addWidget(edit, row, 1);

		QLabel *req = new QLabel(reqText(), parent);
		grid->addWidget(req, row, 2);

		if ( !f.desc().isEmpty() ) {
			QToolTip::add(label, f.desc());
			QToolTip::add(edit, f.desc());
			QToolTip::add(req, f.desc());
		}
	}

	XData::Field field() const
	{
		XData::Field f = XDataField::field();
		QStringList val;
		val << edit->text();
		f.setValue(val);
		return f;
	}

protected:
	QLineEdit *edit;
};

////////////////////////////////////////

class XDataField_TextPrivate : public XDataField_TextSingle
{
public:
	XDataField_TextPrivate(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField_TextSingle(f, grid, row, parent)
	{
		edit->setEchoMode(QLineEdit::Password);
	}
};

////////////////////////////////////////

class XDataField_JidSingle : public XDataField_TextSingle
{
public:
	XDataField_JidSingle(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField_TextSingle(f, grid, row, parent)
	{
		// TODO: add proper validation
	}
};

////////////////////////////////////////

class XDataField_ListSingle : public XDataField
{
public:
	XDataField_ListSingle(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField(f)
	{
		QLabel *label = new QLabel(labelText(), parent);
		grid->addWidget(label, row, 0);

		combo = new QComboBox(parent);
		grid->addWidget(combo, row, 1);
		combo->setInsertionPolicy(QComboBox::NoInsertion);

		QString sel;
		if ( !f.value().isEmpty() )
			sel = f.value().first();

		XData::Field::OptionList opts = f.options();
		XData::Field::OptionList::Iterator it = opts.begin();
		for ( ; it != opts.end(); ++it) {
			QString lbl = (*it).label;
			if ( lbl.isEmpty() )
				lbl = (*it).value;

			combo->insertItem(lbl);
			if ( (*it).value == sel )
				combo->setCurrentText( lbl );
		}

		QLabel *req = new QLabel(reqText(), parent);
		grid->addWidget(req, row, 2);

		if ( !f.desc().isEmpty() ) {
			QToolTip::add(label, f.desc());
			QToolTip::add(combo, f.desc());
			QToolTip::add(req, f.desc());
		}
	}

	XData::Field field() const
	{
		QString lbl = combo->currentText();

		XData::Field f = XDataField::field();
		QStringList val;

		XData::Field::OptionList opts = f.options();
		XData::Field::OptionList::Iterator it = opts.begin();
		for ( ; it != opts.end(); ++it) {
			if ( (*it).label == lbl || (*it).value == lbl ) {
				val << (*it).value;
				break;
			}
		}

		f.setValue(val);
		return f;
	}

private:
	QComboBox *combo;
};

////////////////////////////////////////

class XDataField_ListMulti : public XDataField
{
public:
	XDataField_ListMulti(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField(f)
	{
		QLabel *label = new QLabel(labelText(), parent);
		grid->addWidget(label, row, 0);

		list = new QListBox(parent);
		grid->addWidget(list, row, 1);
		list->setSelectionMode(QListBox::Multi);

		QListBoxText *last = 0;
		XData::Field::OptionList opts = f.options();
		XData::Field::OptionList::Iterator it = opts.begin();
		for ( ; it != opts.end(); ++it) {
			QString lbl = (*it).label;
			if ( lbl.isEmpty() )
				lbl = (*it).value;

			QListBoxText *item = new QListBoxText(list, lbl, last);

			QStringList val = f.value();
			QStringList::Iterator sit = val.begin();
			for ( ; sit != val.end(); ++sit)
				if ( (*it).label == *sit || (*it).value == *sit )
					list->setSelected(item, true);

			last = item;
		}

		QLabel *req = new QLabel(reqText(), parent);
		grid->addWidget(req, row, 2);

		if ( !f.desc().isEmpty() ) {
			QToolTip::add(label, f.desc());
			QToolTip::add(list, f.desc());
			QToolTip::add(req, f.desc());
		}
	}

	XData::Field field() const
	{
		XData::Field f = XDataField::field();
		QStringList val;

		QListBoxItem *item = list->firstItem();
		while ( item ) {
			if ( item->isSelected() ) {
				QString lbl = item->text();
				XData::Field::OptionList opts = f.options();
				XData::Field::OptionList::Iterator it = opts.begin();
				for ( ; it != opts.end(); ++it) {
					if ( (*it).label == lbl || (*it).value == lbl ) {
						val << (*it).value;
						break;
					}
				}
			}

			item = item->next();
		}

		f.setValue(val);
		return f;
	}

private:
	QListBox *list;
};

////////////////////////////////////////

class XDataField_TextMulti : public XDataField
{
public:
	XDataField_TextMulti(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField(f)
	{
		QLabel *label = new QLabel(labelText(), parent);
		grid->addWidget(label, row, 0);

		edit = new QTextEdit(parent);
		grid->addWidget(edit, row, 1);

		QString text;
		QStringList val = f.value();
		QStringList::Iterator it = val.begin();
		for ( ; it != val.end(); ++it) {
			if ( !text.isEmpty() )
				text += "\n";
			text += *it;
		}
		edit->setText(text);

		QLabel *req = new QLabel(reqText(), parent);
		grid->addWidget(req, row, 2);

		if ( !f.desc().isEmpty() ) {
			QToolTip::add(label, f.desc());
			QToolTip::add(edit, f.desc());
			QToolTip::add(req, f.desc());
		}
	}

	XData::Field field() const
	{
		XData::Field f = XDataField::field();
		f.setValue( QStringList::split("\n", edit->text(), true) );
		return f;
	}

private:
	QTextEdit *edit;
};

////////////////////////////////////////

class XDataField_JidMulti : public XDataField_TextMulti
{
public:
	XDataField_JidMulti(XData::Field f, QGridLayout *grid, int row, QWidget *parent)
	: XDataField_TextMulti(f, grid, row, parent)
	{
		// TODO: improve validation
	}
};

//----------------------------------------------------------------------------
// XDataWidget
//----------------------------------------------------------------------------
class XDataWidget::Private
{
public:
	Private() {
		fields.setAutoDelete(true);
	}

	typedef QPtrList<XDataField> XDataFieldList;
	XDataFieldList fields;
};

XDataWidget::XDataWidget(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	d = new Private;
}

XDataWidget::~XDataWidget()
{
	delete d;
}

XData::FieldList XDataWidget::fields() const
{
	XData::FieldList f;

	QPtrListIterator<XDataField> it (d->fields);
	XDataField *field;
	for ( ; (field = it.current()); ++it)
		f.append( field->field() );

	return f;
}

void XDataWidget::setFields(const XData::FieldList &f)
{
	d->fields.clear();

	// delete all child widgets
	QObjectList *objlist = queryList();
	objlist->setAutoDelete(true);
	delete objlist;

	if ( f.count() ) {
		QGridLayout *grid = new QGridLayout(this, 3, f.count(), 0, 3);

		int row = 0;
		XData::FieldList::ConstIterator it = f.begin();
		for ( ; it != f.end(); ++it, ++row) {
			XDataField *f;
			switch ( (*it).type() ) {
				case XData::Field::Field_Boolean:
					f = new XDataField_Boolean(*it, grid, row, this);
					break;
				case XData::Field::Field_Fixed:
					f = new XDataField_Fixed(*it, grid, row, this);
					break;
				case XData::Field::Field_Hidden:
					f = new XDataField_Hidden(*it);
					break;
				case XData::Field::Field_JidSingle:
					f = new XDataField_JidSingle(*it, grid, row, this);
					break;
				case XData::Field::Field_ListMulti:
					f = new XDataField_ListMulti(*it, grid, row, this);
					break;
				case XData::Field::Field_ListSingle:
					f = new XDataField_ListSingle(*it, grid, row, this);
					break;
				case XData::Field::Field_TextMulti:
					f = new XDataField_TextMulti(*it, grid, row, this);
					break;
				case XData::Field::Field_JidMulti:
					f = new XDataField_JidMulti(*it, grid, row, this);
					break;
				case XData::Field::Field_TextPrivate:
					f = new XDataField_TextPrivate(*it, grid, row, this);
					break;

				default:
					f = new XDataField_TextSingle(*it, grid, row, this);
			}
			d->fields.append(f);
		}
	}
}
