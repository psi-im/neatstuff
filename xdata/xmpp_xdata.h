#ifndef XMPPXDATA_H
#define XMPPXDATA_H

#include <qvaluelist.h>

#include "xmpp_tasks.h"

namespace Jabber {

	class XData
	{
	public:
		XData();
		XData(const XData &);
		~XData();

		QString title() const;
		void setTitle(const QString &);

		QString instructions() const;
		void setInstructions(const QString &);

		enum Type {
			Data_Form,
			Data_Result,
			Data_Submit,
			Data_Cancel
		};

		Type type() const;
		void setType(Type);

		void fromXml(const QDomElement &);
		QDomElement toXml(QDomDocument *, bool submitForm = true);

	public:
		class Field {
		public:
			Field();
			~Field();

			QString desc() const;
			void setDesc(const QString &);

			struct Option {
				QString label;
				QString value;
			};

			typedef QValueList<Option> OptionList;
			OptionList options() const;
			void setOptions(OptionList);

			bool required() const;
			void setRequired(bool);

			QString label() const;
			void setLabel(const QString &);

			QString var() const;
			void setVar(const QString &);

			// generic value variable, because every possible Type
			// can be converted to QStringList. Field_Single will
			// use just one string in QStringList. Field_Boolean will
			// use just one string, and that string will equal 0 or 1.
			// and so on...
			QStringList value() const;
			void setValue(const QStringList &);

			enum Type {
				Field_Boolean,
				Field_Fixed,
				Field_Hidden,
				Field_JidMulti,
				Field_JidSingle,
				Field_ListMulti,
				Field_ListSingle,
				Field_TextMulti,
				Field_TextPrivate,
				Field_TextSingle
			};

			Type type() const;
			void setType(Type);

			bool isValid() const;

			void fromXml(const QDomElement &);
			QDomElement toXml(QDomDocument *, bool submitForm = true);

		private:
			QString _desc, _label, _var;
			QValueList<Option> _options;
			bool _required;
			Type _type;
			QStringList _value;
		};

		typedef QValueList<Field> FieldList;

		FieldList fields() const;
		void setFields(const FieldList &);

		// Field* class should set correct Type in constructor
		// and initialize default parameters
		/*class FieldBoolean : public Field {
		public:
			FieldBoolean();
			~FieldBoolean();

			bool valueBoolean() const;
			void setValueBoolean(bool);
		};

		class FieldTextSingle : public Field {
		public:
			FieldTextSingle();
			~FielTextSingle();

			QString valueTextSingle() const;
			void setValueTextSingle(const QString &);
		};

		class FieldTextPrivate : public Field {
		public:
			FieldTextPrivate();
			~FielTextPrivate();
		};

		class FieldTextMulti : public FieldTextSingle {
		public:
			FieldTextMulti();
			~FielTextMulti();

			QString valueTextMulti() const;
			void setValueTextMulti(const QString &);
		};

		class FieldFixed : public FieldTextSingle {
		public:
			FieldFixed();
			~FielFixed();

			QString valueFixed() const;
			void setValueFixed(const QString &);
		};

		class FieldHidden : public FieldTextSingle {
		public:
			FieldHidden();
			~FielHidden();

			QString valueHidden() const;
			void setValueHidden(const QString &);
		};

		class FieldJidSingle : public Field {
		public:
			FieldJidSingle();
			~FieldJidSingle();

			Jid valueJidSingle() const;
			void setValueJidSingle(const Jid &);
		};

		class FieldJidMulti : public FieldJidSingle {
		public:
			FieldJidMulti();
			~FieldJidMulti();

			JidList valueJidMulti() const;
			void setValueJidMulti(const JidList &);
		};

		class FieldListSingle : public FieldTextSingle {
		public:
			FieldListSingle();
			~FieldListSingle();

			QString valueListSingle() const;
			void setValueListSingle(const QString &);
		};

		class FieldListMulti : public FieldListSingle {
		public:
			FieldListMulti();
			~FieldListMulti();

			QStringList valueListMulti() const;
			void setValueListMulti(const QStringList &);
		};*/

		XData &operator= (const XData &);
		XData copy() const;
		void detach();

	private:
		class Private;
		Private *d;
	};

};

#endif
