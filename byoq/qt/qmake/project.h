/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the qmake application of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROJECT_H
#define PROJECT_H

#include <qstringlist.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qstack.h>
#include <qmap.h>

class QMakeProperty;

struct ParsableBlock;
struct IteratorBlock;
struct FunctionBlock;

class QMakeProject
{
    struct ScopeBlock
    {
        enum TestStatus { TestNone, TestFound, TestSeek };
        ScopeBlock() : iterate(0), ignore(false), else_status(TestNone) { }
        ScopeBlock(bool i) : iterate(0), ignore(i), else_status(TestNone) { }
        ~ScopeBlock();
        IteratorBlock *iterate;
        uint ignore : 1, else_status : 2;
    };
    friend struct ParsableBlock;
    friend struct IteratorBlock;
    friend struct FunctionBlock;

    QStack<ScopeBlock> scope_blocks;
    QStack<FunctionBlock *> function_blocks;
    IteratorBlock *iterator;
    FunctionBlock *function;
    QMap<QString, FunctionBlock*> testFunctions, replaceFunctions;

    bool own_prop;
    QString pfile, cfile;
    QMakeProperty *prop;
    void reset();
    QMap<QString, QStringList> vars, base_vars, cache;
    bool parse(const QString &text, QMap<QString, QStringList> &place);

    enum IncludeStatus {
        IncludeSuccess,
        IncludeFeatureAlreadyLoaded,
        IncludeFailure,
        IncludeNoExist,
        IncludeParseFailure
    };
    enum IncludeFlags {
        IncludeFlagNone = 0x00,
        IncludeFlagFeature = 0x01,
        IncludeFlagNewProject = 0x02
    };
    IncludeStatus doProjectInclude(QString file, uchar flags, QMap<QString, QStringList> &place);
    bool doProjectTest(QString str, QMap<QString, QStringList> &place);
    bool doProjectTest(QString func, const QString &params,
                       QMap<QString, QStringList> &place);
    bool doProjectTest(QString func, QStringList args,
                       QMap<QString, QStringList> &place);
    QString doProjectExpand(QString func, const QString &params,
                            QMap<QString, QStringList> &place);
    QString doProjectExpand(QString func, QStringList args,
                            QMap<QString, QStringList> &place);

    bool doProjectCheckReqs(const QStringList &deps, QMap<QString, QStringList> &place);
    bool doVariableReplace(QString &str, QMap<QString, QStringList> &place);
    void init(QMakeProperty *, const QMap<QString, QStringList> *);

public:
    QMakeProject() { init(0, 0); }
    QMakeProject(QMakeProperty *p) { init(p, 0); }
    QMakeProject(const QMap<QString, QStringList> &nvars) { init(0, &nvars); }
    QMakeProject(QMakeProperty *p, const QMap<QString, QStringList> &nvars) { init(p, &nvars); }
    ~QMakeProject();

    enum { ReadCache=0x01, ReadConf=0x02, ReadCmdLine=0x04, ReadProFile=0x08,
           ReadPostFiles=0x10, ReadFeatures=0x20, ReadConfigs=0x40, ReadAll=0xFF };
    inline bool parse(const QString &text) { return parse(text, vars); }
    bool read(const QString &project, uchar cmd=ReadAll);
    bool read(uchar cmd=ReadAll);

    QString projectFile();
    QString configFile();
    inline QMakeProperty *properities() { return prop; }

    QString expand(const QString &v);
    bool test(const QString &v);
    bool test(const QString &func, const QStringList &args);
    bool isActiveConfig(const QString &x, bool regex=false,
                        QMap<QString, QStringList> *place=NULL);

    bool isEmpty(const QString &v);
    QStringList &values(const QString &v);
    QString first(const QString &v);
    QMap<QString, QStringList> &variables();

protected:
    friend class MakefileGenerator;
    bool read(const QString &file, QMap<QString, QStringList> &place);
    bool read(QTextStream &file, QMap<QString, QStringList> &place);

};

inline QString QMakeProject::projectFile()
{
    if (pfile == "-")
        return QString("(stdin)");
    return pfile;
}

inline QString QMakeProject::configFile()
{ return cfile; }

inline bool QMakeProject::isEmpty(const QString &v)
{ return !vars.contains(v) || vars[v].isEmpty(); }

inline QStringList &QMakeProject::values(const QString &v)
{ return vars[v]; }

inline QString QMakeProject::first(const QString &v)
{
    if (isEmpty(v))
        return QString("");
    return vars[v].first();
}

inline QMap<QString, QStringList> &QMakeProject::variables()
{ return vars; }

#endif // PROJECT_H
