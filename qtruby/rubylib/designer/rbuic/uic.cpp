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

#include "uic.h"
#include "ui4.h"
#include "driver.h"
#include "option.h"

// operations
#include "treewalker.h"
#include "validator.h"
//#include "writeincludes.h"
#include "writedeclaration.h"

#include <qdom.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>

#if defined Q_WS_WIN
#include <qt_windows.h>
#endif

Uic::Uic(Driver *d)
     : drv(d),
       out(d->output()),
       opt(d->option()),
       info(d),
       cWidgetsInfo(d),
       externalPix(true)
{
}

Uic::~Uic()
{
}

bool Uic::printDependencies()
{
    QString fileName = opt.inputFile;

    QFile f;
    if (fileName.isEmpty())
        f.open(stdin, QIODevice::ReadOnly);
    else {
        f.setFileName(fileName);
        if (!f.open(QIODevice::ReadOnly))
            return false;
    }

    QDomDocument doc;                        // ### generalize. share more code with the other tools!
    if (!doc.setContent(&f))
        return false;

    QDomElement root = doc.firstChild().toElement();
    DomUI *ui = new DomUI();
    ui->read(root);

    double version = ui->attributeVersion().toDouble();
    if (version < 4.0) {
        delete ui;

        fprintf(stderr, "uic: File generated with too old version of Qt Designer\n");
        return false;
    }

    if (DomIncludes *includes = ui->elementIncludes()) {
        foreach (DomInclude *incl, includes->elementInclude()) {
            QString file = incl->text();
            if (file.isEmpty())
                continue;

            fprintf(stdout, "%s\n", file.toLocal8Bit().constData());
        }
    }

    if (DomCustomWidgets *customWidgets = ui->elementCustomWidgets()) {
        foreach (DomCustomWidget *customWidget, customWidgets->elementCustomWidget()) {
            if (DomHeader *header = customWidget->elementHeader()) {
                QString file = header->text();
                if (file.isEmpty())
                    continue;

                fprintf(stdout, "%s\n", file.toLocal8Bit().constData());
            }
        }
    }

    delete ui;

    return true;
}

void Uic::writeCopyrightHeader(DomUI *ui)
{
    QString comment = ui->elementComment();
    if (comment.size())
        out << "=begin\n" << comment << "\n=end\n";
}

bool Uic::write(QIODevice *in)
{
    QDomDocument doc;
    if (!doc.setContent(in))
        return false;

    QDomElement root = doc.firstChild().toElement();
    DomUI *ui = new DomUI();
    ui->read(root);

    double version = ui->attributeVersion().toDouble();
    if (version < 4.0) {
        delete ui;

        fprintf(stderr, "uic: File generated with too old version of Qt Designer\n");
        return false;
    }

    bool rtn = write(ui);
    delete ui;

    return rtn;
}

bool Uic::write(DomUI *ui)
{
    if (!ui || !ui->elementWidget())
        return false;

    if (opt.copyrightHeader)
        writeCopyrightHeader(ui);

    pixFunction = ui->elementPixmapFunction();
    if (pixFunction == QLatin1String("Qt::Pixmap.fromMimeSource"))
        pixFunction = QLatin1String("qPixmapFromMimeSource");

    externalPix = ui->elementImages() == 0;

    info.acceptUI(ui);
    cWidgetsInfo.acceptUI(ui);
//    WriteIncludes(this).acceptUI(ui);

    Validator(this).acceptUI(ui);
    WriteDeclaration(this).acceptUI(ui);

    return true;
}

//void Uic::writeHeaderProtectionStart()
//{
//    QString h = drv->headerFileName();
//    out << "#ifndef " << h << "\n"
//        << "#define " << h << "\n";
//}

//void Uic::writeHeaderProtectionEnd()
//{
//    QString h = drv->headerFileName();
//    out << "#endif // " << h << "\n";
//}

bool Uic::isMainWindow(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("Q3MainWindow"))
        || customWidgetsInfo()->extends(className, QLatin1String("QMainWindow"));
}

bool Uic::isToolBar(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("Q3ToolBar"))
        || customWidgetsInfo()->extends(className, QLatin1String("QToolBar"));
}

bool Uic::isButton(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QRadioButton"))
        || customWidgetsInfo()->extends(className, QLatin1String("QToolButton"))
        || customWidgetsInfo()->extends(className, QLatin1String("QCheckBox"))
        || customWidgetsInfo()->extends(className, QLatin1String("QPushButton"));
}

bool Uic::isContainer(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QStackedWidget"))
        || customWidgetsInfo()->extends(className, QLatin1String("QToolBox"))
        || customWidgetsInfo()->extends(className, QLatin1String("QTabWidget"));
}

bool Uic::isStatusBar(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QStatusBar"));
}

bool Uic::isMenuBar(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QMenuBar"));
}

bool Uic::isMenu(const QString &className) const
{
    return customWidgetsInfo()->extends(className, QLatin1String("QMenu"))
        || customWidgetsInfo()->extends(className, QLatin1String("QPopupMenu"));
}
