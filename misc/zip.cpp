#include"zip.h"

#include<qcstring.h>
#include<qstringlist.h>
#include<qfile.h>

#include"minizip/unzip.h"

class UnZipPrivate
{
public:
	UnZipPrivate() {}

	QString name;
	unzFile uf;
	QStringList listing;
};

UnZip::UnZip(const QString &name)
{
	d = new UnZipPrivate;
	d->uf = 0;
	d->name = name;
}

UnZip::~UnZip()
{
	close();
	delete d;
}

void UnZip::setName(const QString &name)
{
	d->name = name;
}

const QString & UnZip::name() const
{
	return d->name;
}

bool UnZip::open()
{
	close();

	d->uf = unzOpen(QFile::encodeName(d->name).data());
	if(!d->uf)
		return false;

	return getList();
}

void UnZip::close()
{
	if(d->uf) {
		unzClose(d->uf);
		d->uf = 0;
	}

	d->listing.clear();
}

const QStringList & UnZip::list() const
{
	return d->listing;
}

bool UnZip::getList()
{
	unz_global_info gi;
	int err = unzGetGlobalInfo(d->uf, &gi);
	if(err != UNZ_OK)
		return false;

	QStringList l;
	for(int n = 0; n < (int)gi.number_entry; ++n) {
		char filename_inzip[256];
		unz_file_info file_info;
		int err = unzGetCurrentFileInfo(d->uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		if(err != UNZ_OK)
			return false;

		l += filename_inzip;

		if((n+1) < (int)gi.number_entry) {
			err = unzGoToNextFile(d->uf);
			if(err != UNZ_OK)
				return false;
		}
	}

	d->listing = l;

	return true;
}

bool UnZip::readFile(const QString &fname, QByteArray *buf, int max)
{
	int err = unzLocateFile(d->uf, QFile::encodeName(fname).data(), 0);
	if(err != UNZ_OK)
		return false;

	char filename_inzip[256];
	unz_file_info file_info;
	err = unzGetCurrentFileInfo(d->uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
	if(err != UNZ_OK)
		return false;

	err = unzOpenCurrentFile(d->uf);
	if(err != UNZ_OK)
		return false;

	QByteArray a(0);
	QByteArray chunk(16384);
	while(1) {
		err = unzReadCurrentFile(d->uf, chunk.data(), chunk.size());
		if(err < 0) {
			unzCloseCurrentFile(d->uf);
			return false;
		}
		if(err == 0)
			break;

		int oldsize = a.size();
		if(max > 0 && oldsize + err > max)
			err = max - oldsize;
		a.resize(oldsize + err);
		memcpy(a.data() + oldsize, chunk.data(), err);

		if(max > 0 && (int)a.size() >= max)
			break;
	}

	err = unzCloseCurrentFile(d->uf);
	if(err != UNZ_OK)
		return false;

	*buf = a;
	return true;
}
