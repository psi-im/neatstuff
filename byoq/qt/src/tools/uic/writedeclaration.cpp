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

#include "writedeclaration.h"
#include "writeicondeclaration.h"
#include "writeinitialization.h"
#include "writeiconinitialization.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"
#include "databaseinfo.h"
#include "customwidgetsinfo.h"

#include <QTextStream>

WriteDeclaration::WriteDeclaration(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
    this->uic = uic;
}

void WriteDeclaration::acceptUI(DomUI *node)
{
    QString qualifiedClassName = node->elementClass() + option.postfix;
    QString className = qualifiedClassName;

    QString varName = driver->findOrInsertWidget(node->elementWidget());
    QString widgetClassName = node->elementWidget()->attributeClass();

    QString exportMacro = node->elementExportMacro();
    if (!exportMacro.isEmpty())
        exportMacro.append(QLatin1Char(' '));

    QStringList nsList = qualifiedClassName.split(QLatin1String("::"));
    if (nsList.count()) {
        className = nsList.last();
        nsList.removeLast();
    }

    QListIterator<QString> it(nsList);
    while (it.hasNext()) {
        QString ns = it.next();
        if (ns.isEmpty())
            continue;

        output << "namespace " << ns << " {\n";
    }

    if (nsList.count())
        output << "\n";

    output << "class " << exportMacro << option.prefix << className << "\n"
           << "{\n"
           << "public:\n";

    QStringList connections = uic->databaseInfo()->connections();
    for (int i=0; i<connections.size(); ++i) {
        QString connection = connections.at(i);

        if (connection == QLatin1String("(default)"))
            continue;

        output << option.indent << "QSqlDatabase " << connection << "Connection;\n";
    }

    TreeWalker::acceptWidget(node->elementWidget());

    output << "\n";

    WriteInitialization(uic).acceptUI(node);

    if (node->elementImages()) {
        output << "\n"
            << "protected:\n"
            << option.indent << "enum IconID\n"
            << option.indent << "{\n";
        WriteIconDeclaration(uic).acceptUI(node);

        output << option.indent << option.indent << "unknown_ID\n"
            << option.indent << "};\n";

        WriteIconInitialization(uic).acceptUI(node);
    }

    output << "};\n\n";

    it.toBack();
    while (it.hasPrevious()) {
        QString ns = it.previous();
        if (ns.isEmpty())
            continue;

        output << "} // namespace " << ns << "\n";
    }

    if (nsList.count())
        output << "\n";

    if (option.generateNamespace && !option.prefix.isEmpty()) {
        nsList.append(QLatin1String("Ui"));

        QListIterator<QString> it(nsList);
        while (it.hasNext()) {
            QString ns = it.next();
            if (ns.isEmpty())
                continue;

            output << "namespace " << ns << " {\n";
        }

        output << option.indent << "class " << exportMacro << className << ": public " << option.prefix << className << " {};\n";

        it.toBack();
        while (it.hasPrevious()) {
            QString ns = it.previous();
            if (ns.isEmpty())
                continue;

            output << "} // namespace " << ns << "\n";
        }

        if (nsList.count())
            output << "\n";
    }
}

void WriteDeclaration::acceptWidget(DomWidget *node)
{
    QString className = QLatin1String("QWidget");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    output << option.indent << uic->customWidgetsInfo()->realClassName(className) << " *" << driver->findOrInsertWidget(node) << ";\n";

    TreeWalker::acceptWidget(node);
}

void WriteDeclaration::acceptLayout(DomLayout *node)
{
    QString className = QLatin1String("QLayout");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    output << option.indent << className << " *" << driver->findOrInsertLayout(node) << ";\n";

    TreeWalker::acceptLayout(node);
}

void WriteDeclaration::acceptSpacer(DomSpacer *node)
{
    output << option.indent << "QSpacerItem *" << driver->findOrInsertSpacer(node) << ";\n";

    TreeWalker::acceptSpacer(node);
}

void WriteDeclaration::acceptActionGroup(DomActionGroup *node)
{
    output << option.indent << "QActionGroup *" << driver->findOrInsertActionGroup(node) << ";\n";

    TreeWalker::acceptActionGroup(node);
}

void WriteDeclaration::acceptAction(DomAction *node)
{
    output << option.indent << "QAction *" << driver->findOrInsertAction(node) << ";\n";

    TreeWalker::acceptAction(node);
}
