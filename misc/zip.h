#ifndef ZIP_H
#define ZIP_H

#include<qstring.h>
#include<qcstring.h>

class QStringList;

class UnZip
{
public:
	UnZip(const QString &fname="");
	~UnZip();

	void setName(const QString &);
	const QString & name() const;

	bool open();
	void close();

	const QStringList & list() const;
	bool readFile(const QString &, QByteArray *, int max=0);

private:
	class UnZipPrivate *d;

	bool getList();
};

#endif
