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

#include "preprocessor.h"
#include "scanner.h"
#include "moc.h"
#include "outputrevision.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*
    This function looks at two file names and returns the name of the
    infile with a path relative to outfile.

    Examples:

        /tmp/abc, /tmp/bcd -> abc
        xyz/a/bc, xyz/b/ac -> ../a/bc
        /tmp/abc, xyz/klm -> /tmp/abc
 */

static QByteArray combinePath(const char *infile, const char *outfile)
{
    QFileInfo inFileInfo(QDir::current(), QFile::decodeName(infile));
    QFileInfo outFileInfo(QDir::current(), QFile::decodeName(outfile));
    int numCommonComponents = 0;

    QStringList inSplitted = inFileInfo.dir().canonicalPath().split('/');
    QStringList outSplitted = outFileInfo.dir().canonicalPath().split('/');

    while (!inSplitted.isEmpty() && !outSplitted.isEmpty() &&
            inSplitted.first() == outSplitted.first()) {
        inSplitted.removeFirst();
        outSplitted.removeFirst();
        numCommonComponents++;
    }

    if (numCommonComponents < 2)
        /*
          The paths don't have the same drive, or they don't have the
          same root directory. Use an absolute path.
        */
        return QFile::encodeName(inFileInfo.absoluteFilePath());
    /*
       The paths have something in common. Use a path relative to
       the output file.
     */
    while (!outSplitted.isEmpty()) {
        outSplitted.removeFirst();
        inSplitted.prepend("..");
    }
    inSplitted.append(inFileInfo.fileName());
    return QFile::encodeName(inSplitted.join("/"));
}


void error(const char *msg = "Invalid argument")
{
    if (msg)
        fprintf(stderr, "moc: %s\n", msg);
    fprintf(stderr, "Usage: moc [options] <header-file>\n"
            "    -o<file>           Write output to file rather than stdout\n"
            "    -I<dir>            Add dir to the include path for header files\n"
            "    -E                 Preprocess only; do not generate meta object code\n"
            "    -D<macro>[=<def>]  Define macro, with optional definition\n"
            "    -U<macro>          Undefine macro\n"
            "    -i                 Do not generate an #include statement\n"
            "    -p<path>           Path prefix for included file\n"
            "    -f[<file>]         Force #include, optional file name\n"
            "    -nw                Do not display warnings\n"
            "    -v                 Display version of moc\n");
    exit(1);
}

int main(int argc, char **argv)
{
    bool autoInclude = true;
    Preprocessor::macros["Q_MOC_RUN"] = "";
    Preprocessor::macros["__cplusplus"] = "";
    Moc moc;
    QByteArray filename;
    QByteArray output;
    FILE *in = 0;
    FILE *out = 0;
    bool ignoreConflictingOptions = false;
    for (int n = 1; n < argc; ++n) {
        QByteArray arg(argv[n]);
        if (arg[0] != '-') {
            if (filename.isEmpty()) {
                filename = arg;
                continue;
            }
            error("Too many input files specified");
        }
        QByteArray opt = arg.mid(1);
        bool more = (opt.size() > 1);
        switch (opt[0]) {
        case 'o': // output redirection
            if (!more) {
                if (!(n < argc-1))
                    error("Missing output file name");
                output = argv[++n];
            } else
                output = opt.mid(1);
            break;
        case 'E': // only preprocessor
            Preprocessor::onlyPreprocess = true;
            break;
        case 'i': // no #include statement
            if (more)
                error();
            moc.noInclude        = true;
            autoInclude = false;
            break;
        case 'f': // produce #include statement
            if (ignoreConflictingOptions)
                break;
            moc.noInclude        = false;
            autoInclude = false;
            if (opt[1])                        // -fsomething.h
                moc.includeFiles.append(opt.mid(1));
            break;
        case 'p': // include file path
            if (ignoreConflictingOptions)
                break;
            if (!more) {
                if (!(n < argc-1))
                    error("Missing path name for the -p option.");
                moc.includePath = argv[++n];
            } else {
                moc.includePath = opt.mid(1);
            }
            break;
        case 'I': // produce #include statement
            if (!more) {
                if (!(n < argc-1))
                    error("Missing path name for the -I option.");
                Preprocessor::includes += argv[++n];
            } else {
                Preprocessor::includes += opt.mid(1);
            }
            break;
        case 'D': // define macro
            {
                QByteArray macro, value;
                if (!more) {
                    if (n < argc-1)
                        macro = argv[++n];
                } else
                    macro = opt.mid(1);
                int eq = macro.indexOf('=');
                if (eq >= 0) {
                    value = macro.mid(eq + 1);
                    macro = macro.left(eq);
                }
                if (macro.isEmpty())
                    error("Missing macro name");
                Preprocessor::macros[macro] = value;

            }
            break;
        case 'U':
            {
                QByteArray macro;
                if (!more) {
                    if (n < argc-1)
                        macro = argv[++n];
                } else
                    macro = opt.mid(1);
                if (macro.isEmpty())
                    error("Missing macro name");
                Preprocessor::macros.remove(macro);

            }
            break;
        case 'v':  // version number
            if (more)
                error();
            fprintf(stderr, "Qt Meta Object Compiler version %d (Qt %s)\n",
                    mocOutputRevision, QT_VERSION_STR);
            return 1;
        case 'n': // don't display warnings
            if (ignoreConflictingOptions)
                break;
            if (opt != "nw")
                error();
            moc.displayWarnings = false;
            break;
        case 'h': // help
            error(0); // 0 means usage only
            break;
        case '-': 
            if (more && arg == "--ignore-option-clashes") {
                // -- ignore all following moc specific options that conflict
                // with for example gcc, like -pthread conflicting with moc's
                // -p option.
                ignoreConflictingOptions = true;
                break;
            }
            // fall through
        default:
            error();
        }
    }


    if (autoInclude) {
        int ppos = filename.lastIndexOf('.');
        moc.noInclude = (ppos >= 0 
                         && tolower(filename[ppos + 1]) != 'h'
                         && tolower(filename[ppos + 1]) != QDir::separator().toLatin1()
                        );
    }
    if (moc.includeFiles.isEmpty()) {
        if (moc.includePath.isEmpty()) {
            if (filename.size()) {
                if (output.size())
                    moc.includeFiles.append(combinePath(filename, output));
                else
                    moc.includeFiles.append(filename);
            }
        } else {
            moc.includeFiles.append(combinePath(filename, filename));
        }
    }

    if (filename.isEmpty()) {
        filename = "standard input";
        in = stdin;
    } else {
#if defined(_MSC_VER) && _MSC_VER >= 1400
		if (fopen_s(&in, filename.data(), "r")) {
#else
        in = fopen(filename.data(), "r");
		if (!in) {
#endif
            fprintf(stderr, "moc: %s: No such file\n", (const char*)filename);
            return 1;
        }
        moc.filename = filename;
    }

    // 1. preprocess
    QByteArray input = Preprocessor::preprocessed(moc.filename, in);
    fclose(in);

    if (Preprocessor::onlyPreprocess)
        goto step4;

    // 2. tokenize
    moc.symbols = Scanner::scan(input);

    // 3. parse
    moc.parse();

    // 4. and output meta object code
 step4:

    if (output.size()) { // output file specified
#if defined(_MSC_VER) && _MSC_VER >= 1400
		if (fopen_s(&out, output.data(), "w")) {
#else
        out = fopen(output.data(), "w"); // create output file
        if (!out) {
#endif
            fprintf(stderr, "moc: Cannot create %s\n", (const char*)output);
            return 1;
        }
    } else { // use stdout
        out = stdout;
    }

    if (Preprocessor::onlyPreprocess) {
        fprintf(out, "%s%s\n", Preprocessor::protocol.constData(), input.constData());
    } else {
        if (moc.classList.isEmpty())
            moc.warning("No relevant classes found. No output generated.");
        else
            moc.generate(out);
    }

    if (output.size())
        fclose(out);

    return 0;
}
