#include "xmpp_xdata.h"

#include <qshared.h>

#include "xmpp_jid.h" // for JID validation

using namespace Jabber;

// TODO: report, item

static QDomElement textTag(QDomDocument *doc, const QString &name, const QString &content)
{
	QDomElement tag = doc->createElement(name);
	QDomText text = doc->createTextNode(content);
	tag.appendChild(text);

	return tag;
}

static QDomElement findSubTag(const QDomElement &e, const QString &name, bool *found)
{
	if(found)
		*found = FALSE;

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		if(i.tagName() == name) { // mblsha: ignore case when searching
			if(found)
				*found = TRUE;
			return i;
		}
	}

	QDomElement tmp;
	return tmp;
}

// mblsha's own functions

static QDomElement emptyTag(QDomDocument *doc, const QString &name)
{
	QDomElement tag = doc->createElement(name);

	return tag;
}

static bool hasSubTag(const QDomElement &e, const QString &name)
{
	bool found;
	findSubTag(e, name, &found);
	return found;
}

static QString subTagText(const QDomElement &e, const QString &name)
{
	bool found;
	QDomElement i = findSubTag(e, name, &found);
	if ( found )
		return i.text();
	return QString::null;
}

//----------------------------------------------------------------------------
// XData::Field
//----------------------------------------------------------------------------
XData::Field::Field()
{
}

XData::Field::~Field()
{
}

QString XData::Field::desc() const
{
	return _desc;
}

void XData::Field::setDesc(const QString &d)
{
	_desc = d;
}

XData::Field::OptionList XData::Field::options() const
{
	return _options;
}

void XData::Field::setOptions(XData::Field::OptionList o)
{
	_options = o;
}

bool XData::Field::required() const
{
	return _required;
}

void XData::Field::setRequired(bool r)
{
	_required = r;
}

QString XData::Field::label() const
{
	return _label;
}

void XData::Field::setLabel(const QString &l)
{
	_label = l;
}

QString XData::Field::var() const
{
	return _var;
}

void XData::Field::setVar(const QString &v)
{
	_var = v;
}

QStringList XData::Field::value() const
{
	return _value;
}

void XData::Field::setValue(const QStringList &v)
{
	_value = v;
}

XData::Field::Type XData::Field::type() const
{
	return _type;
}

void XData::Field::setType(XData::Field::Type t)
{
	_type = t;
}

bool XData::Field::isValid() const
{
	if ( _required && _value.isEmpty() )
		return false;

	if ( _type == Field_Boolean ) {
		if ( _value.count() != 1 )
			return false;

		QString str = _value.first();
		if ( str == "0" || str == "1" || str == "true" || str == "false" || str == "yes" || str == "no" )
			return true;
	}
	if ( _type == Field_TextSingle || _type == Field_TextPrivate ) {
		if ( _value.count() == 1 )
			return true;
	}
	if ( _type == Field_JidSingle ) {
		if ( _value.count() != 1 )
			return false;

		Jid j( _value.first() );
		return j.isValid();
	}
	if ( _type == Field_JidMulti ) {
		QStringList::ConstIterator it = _value.begin();
		bool allValid = false;
		for ( ; it != _value.end(); ++it) {
			Jid j(*it);
			if ( !j.isValid() ) {
				allValid = false;
				break;
			}
			return allValid;
		}
	}

	return false;
}

void XData::Field::fromXml(const QDomElement &e)
{
	if ( e.tagName() != "field" )
		return;

	_var = e.attribute("var");
	_label = e.attribute("label");

	QString type = e.attribute("type");
	if ( type == "boolean" )
		_type = Field_Boolean;
	else if ( type == "fixed" )
		_type = Field_Fixed;
	else if ( type == "hidden" )
		_type = Field_Hidden;
	else if ( type == "jid-multi" )
		_type = Field_JidMulti;
	else if ( type == "jid-single" )
		_type = Field_JidSingle;
	else if ( type == "list-multi" )
		_type = Field_ListMulti;
	else if ( type == "list-single" )
		_type = Field_ListSingle;
	else if ( type == "text-multi" )
		_type = Field_TextMulti;
	else if ( type == "text-private" )
		_type = Field_TextPrivate;
	else
		_type = Field_TextSingle;

	_required = false;
	_desc     = QString::null;
	_options.clear();
	_value.clear();

	QDomNode n = e.firstChild();
	for ( ; !n.isNull(); n = n.nextSibling() ) {
		QDomElement i = n.toElement();
		if ( i.isNull() )
			continue;

		QString tag = i.tagName();
		if ( tag == "required" )
			_required = true;
		else if ( tag == "desc" )
			_desc = i.text().simplifyWhiteSpace();
		else if ( tag == "option" ) {
			Option o;
			o.label = i.attribute("label");
			o.value = i.text();
			_options.append(o);
		}
		else if ( tag == "value" ) {
			_value.append(i.text());
		}
	}
}

QDomElement XData::Field::toXml(QDomDocument *doc, bool submitForm)
{
	QDomElement f = doc->createElement("field");

	// setting attributes...
	if ( !_var.isEmpty() )
		f.setAttribute("var", _var);
	if ( !submitForm && !_label.isEmpty() )
		f.setAttribute("label", _label);

	// now we're gonna get the 'type'
	QString type = "text-single";
	if ( _type == Field_Boolean )
		type = "boolean";
	else if ( _type == Field_Fixed )
		type = "fixed";
	else if ( _type == Field_Hidden )
		type = "hidden";
	else if ( _type == Field_JidMulti )
		type = "jid-multi";
	else if ( _type == Field_JidSingle )
		type = "jid-single";
	else if ( _type == Field_ListMulti )
		type = "list-multi";
	else if ( _type == Field_ListSingle )
		type = "list-single";
	else if ( _type == Field_TextMulti )
		type = "text-multi";
	else if ( _type == Field_TextPrivate )
		type = "text-private";

	f.setAttribute("type", type);

	// now, setting nested tags...
	if ( !submitForm && _required )
		f.appendChild( emptyTag(doc, "required") );

	if ( !submitForm && !_desc.isEmpty() )
		f.appendChild( textTag(doc, "desc", _desc) );

	if ( !submitForm && !_options.isEmpty() ) {
		OptionList::Iterator it = _options.begin();
		for ( ; it != _options.end(); ++it) {
			QDomElement o = textTag(doc, "option", (*it).value);
			if ( (*it).label.isEmpty() )
				o.setAttribute("label", (*it).label);

			f.appendChild(o);
		}
	}

	if ( !_value.isEmpty() ) {
		QStringList::Iterator it = _value.begin();
		for ( ; it != _value.end(); ++it)
			f.appendChild( textTag(doc, "value", *it) );
	}

	return f;
}

//----------------------------------------------------------------------------
// XData
//----------------------------------------------------------------------------
class XData::Private : public QShared {
public:
	QString title, instructions;
	XData::Type type;
	FieldList fields;
};

XData::XData()
{
	d = new Private;
}

XData::~XData()
{
	if ( d->deref() )
		delete d;
}

XData::XData(const XData &x)
{
	d = x.d;
	d->ref();
}

QString XData::title() const
{
	return d->title;
}

void XData::setTitle(const QString &t)
{
	detach();
	d->title = t;
}

QString XData::instructions() const
{
	return d->instructions;
}

void XData::setInstructions(const QString &i)
{
	detach();
	d->instructions = i;
}

XData::Type XData::type() const
{
	return d->type;
}

void XData::setType(Type t)
{
	detach();
	d->type = t;
}

XData::FieldList XData::fields() const
{
	return d->fields;
}

void XData::setFields(const FieldList &f)
{
	detach();
	d->fields = f;
}

void XData::fromXml(const QDomElement &e)
{
	if ( e.attribute("xmlns") != "jabber:x:data" )
		return;

	detach();

	QString type = e.attribute("type");
	if ( type == "result" )
		d->type = Data_Result;
	else if ( type == "submit" )
		d->type = Data_Submit;
	else if ( type == "cancel" )
		d->type = Data_Cancel;
	else
		d->type = Data_Form;

	d->title        = subTagText(e, "title");
	d->instructions = subTagText(e, "instructions");

	d->fields.clear();

	QDomNode n = e.firstChild();
	for ( ; !n.isNull(); n = n.nextSibling() ) {
		QDomElement i = n.toElement();
		if ( i.isNull() )
			continue;

		if ( i.tagName() == "field" ) {
			Field f;
			f.fromXml(i);
			d->fields.append(f);
		}
	}
}

QDomElement XData::toXml(QDomDocument *doc, bool submitForm)
{
	QDomElement x = doc->createElement("x");
	x.setAttribute("xmlns", "jabber:x:data");

	QString type = "form";
	if ( d->type == Data_Result )
		type = "result";
	else if ( d->type == Data_Submit )
		type = "submit";
	else if ( d->type == Data_Cancel )
		type = "cancel";

	x.setAttribute("type", type);

	if ( !submitForm && !d->title.isEmpty() )
		x.appendChild( textTag(doc, "title", d->title) );
	if ( !submitForm && !d->instructions.isEmpty() )
		x.appendChild( textTag(doc, "instructions", d->instructions) );

	if ( !d->fields.isEmpty() ) {
		FieldList::Iterator it = d->fields.begin();
		for ( ; it != d->fields.end(); ++it) {
			Field f = *it;
			if ( !(submitForm && f.var().isEmpty()) )
				x.appendChild( f.toXml(doc, submitForm) );
		}
	}

	return x;
}

XData &XData::operator= (const XData &from)
{
	if ( d->deref() )
		delete d;

	d = from.d;
	d->ref();

	return *this;
}

XData XData::copy() const
{
	XData foo;
	delete foo.d;
	foo.d = new Private( *this->d );

	return foo;
}

void XData::detach()
{
	if ( d->count != 1 ) // only if >1 reference
		*this = copy();
}
