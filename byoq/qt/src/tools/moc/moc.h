/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef MOC_H
#define MOC_H

#include "scanner.h"
#include <QStringList>
#include <QMap>
#include <QPair>
#include <QStack>
#include <stdio.h>
#include <ctype.h>

struct Type
{
    enum ReferenceType { NoReference, Reference, Pointer };

    inline Type() : isVolatile(false), referenceType(NoReference) {}
    inline explicit Type(const QByteArray &_name) : name(_name), isVolatile(false), referenceType(NoReference) {}
    QByteArray name;
    bool isVolatile;
    ReferenceType referenceType;
};

struct EnumDef
{
    QByteArray name;
    QList<QByteArray> values;
};

struct ArgumentDef
{
    ArgumentDef() : isDefault(false) {}
    Type type;
    QByteArray rightType, normalizedType, name;
    bool isDefault;
};

struct FunctionDef
{
    FunctionDef(): returnTypeIsVolatile(false), access(Private), isConst(false), isVirtual(false),
                   inlineCode(false), wasCloned(false), isCompat(false), isInvokable(false), 
                   isScriptable(false), isSlot(false), isSignal(false) {}
    Type type;
    QByteArray normalizedType;
    QByteArray tag;
    QByteArray name;
    bool returnTypeIsVolatile;

    QList<ArgumentDef> arguments;

    enum Access { Private, Protected, Public };
    Access access;
    bool isConst;
    bool isVirtual;
    bool inlineCode;
    bool wasCloned;

    QByteArray inPrivateClass;
    bool isCompat;
    bool isInvokable;
    bool isScriptable;
    bool isSlot;
    bool isSignal;
};

struct PropertyDef
{
    PropertyDef():gspec(ValueSpec){}
    QByteArray name, type, read, write, reset, designable, scriptable, editable, stored, user;
    enum Specification  { ValueSpec, ReferenceSpec, PointerSpec };
    Specification gspec;
    bool stdCppSet() const {
        QByteArray s("set");
        s += toupper(name[0]);
        s += name.mid(1);
        return (s == write);
    }
};


struct ClassInfoDef
{
    QByteArray name;
    QByteArray value;
};

struct ClassDef {
    ClassDef():
        hasQObject(false), hasQGadget(false){}
    QByteArray classname;
    QByteArray qualified;
    QList<QPair<QByteArray, FunctionDef::Access> > superclassList;

    struct Interface
    {
        inline explicit Interface(const QByteArray &_className)
            : className(_className) {}
        QByteArray className;
        QByteArray interfaceId;
    };
    QList<QList<Interface> >interfaceList;

    bool hasQObject;
    bool hasQGadget;

    QList<FunctionDef> signalList, slotList, methodList, publicList;
    QList<PropertyDef> propertyList;
    QList<ClassInfoDef> classInfoList;
    QMap<QByteArray, bool> enumDeclarations;
    QList<EnumDef> enumList;
    QMap<QByteArray, QByteArray> flagAliases;

    int begin;
    int end;
};

struct NamespaceDef {
    QByteArray name;
    int begin;
    int end;
};

class Moc
{
public:
    Moc()
        :index(0),
         noInclude(false),
         displayWarnings(true),
         generatedCode(false)
        {}

    QByteArray filename;
    QStack<QByteArray> currentFilenames;
    Symbols symbols;

    int index;

    bool noInclude;
    bool displayWarnings;
    bool generatedCode;
    QByteArray includePath;
    QList<QByteArray> includeFiles;
    QList<ClassDef> classList;
    QMap<QByteArray, QByteArray> interface2IdMap;

    inline bool hasNext() const { return (index < symbols.size()); }
    inline Token next() { return symbols.at(index++).token; }
    bool test(Token);
    void next(Token);
    void next(Token, const char *msg);
    bool until(Token);
    QByteArray lexemUntil(Token);
    inline void prev() {--index;}
    Token lookup(int k = 1);
    inline const Symbol &symbol_lookup(int k = 1) { return symbols.at(index-1+k);}
    inline Token token() { return symbols.at(index-1).token;}
    inline QByteArray lexem() { return symbols.at(index-1).lexem();}
    inline const Symbol &symbol() { return symbols.at(index-1);}

    void error(int rollback);
    void error(const char *msg = 0);
    void warning(const char * = 0);

    void parse();
    void generate(FILE *out);

    bool parseClassHead(ClassDef *def);
    inline bool inClass(const ClassDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inNamespace(const NamespaceDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    Type parseType();

    bool parseEnum(EnumDef *def);

    void parseFunction(FunctionDef *def, bool inMacro = false);
    bool parseMaybeFunction(FunctionDef *def);

    void parseSlots(ClassDef *def, FunctionDef::Access access);
    void parseSignals(ClassDef *def);
    void parseProperty(ClassDef *def);
    void parseEnumOrFlag(ClassDef *def, bool isFlag);
    void parseFlag(ClassDef *def);
    void parseClassInfo(ClassDef *def);
    void parseInterfaces(ClassDef *def);
    void parseDeclareInterface();
    void parseSlotInPrivate(ClassDef *def, FunctionDef::Access access);

    void parseFunctionArguments(FunctionDef *def);

};


inline bool Moc::test(Token token)
{
    if (index < symbols.size() && symbols.at(index).token == token) {
        ++index;
        return true;
    }
    return false;
}

inline Token Moc::lookup(int k)
{
    const int l = index - 1 + k;
    return l < symbols.size() ? symbols.at(l).token : NOTOKEN;
}

inline void Moc::next(Token token)
{
    if (!test(token))
        error();
}

inline void Moc::next(Token token, const char *msg)
{
    if (!test(token))
        error(msg);
}

QByteArray normalizeType(const char *s, bool fixScope = false);

#endif // MOC_H
