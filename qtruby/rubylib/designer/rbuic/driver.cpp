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

#include "driver.h"
#include "uic.h"
#include "ui4.h"

#include <qregexp.h>
#include <qfileinfo.h>
#include <qdebug.h>

Driver::Driver()
    : m_stdout(stdout, QFile::WriteOnly)
{
    m_output = &m_stdout;
}

Driver::~Driver()
{
}

QString Driver::findOrInsertWidget(DomWidget *ui_widget)
{
    if (!m_widgets.contains(ui_widget))
        m_widgets.insert(ui_widget, unique(ui_widget->attributeName(), ui_widget->attributeClass()));

    return m_widgets.value(ui_widget);
}

QString Driver::findOrInsertSpacer(DomSpacer *ui_spacer)
{
    if (!m_spacers.contains(ui_spacer))
        m_spacers.insert(ui_spacer, unique(QString(), QLatin1String("Qt::SpacerItem")));

    return m_spacers.value(ui_spacer);
}

QString Driver::findOrInsertLayout(DomLayout *ui_layout)
{
    if (!m_layouts.contains(ui_layout))
        m_layouts.insert(ui_layout, unique(QString(), ui_layout->attributeClass()));

    return m_layouts.value(ui_layout);
}

QString Driver::findOrInsertLayoutItem(DomLayoutItem *ui_layoutItem)
{
    switch (ui_layoutItem->kind()) {
        case DomLayoutItem::Widget:
            return findOrInsertWidget(ui_layoutItem->elementWidget());
        case DomLayoutItem::Spacer:
            return findOrInsertSpacer(ui_layoutItem->elementSpacer());
        case DomLayoutItem::Layout:
            return findOrInsertLayout(ui_layoutItem->elementLayout());
        case DomLayoutItem::Unknown:
            break;
    }

    Q_ASSERT( 0 );

    return QString();
}

QString Driver::findOrInsertActionGroup(DomActionGroup *ui_group)
{
    if (!m_actionGroups.contains(ui_group))
        m_actionGroups.insert(ui_group, unique(ui_group->attributeName(), QLatin1String("Qt::ActionGroup")));

    return m_actionGroups.value(ui_group);
}

QString Driver::findOrInsertAction(DomAction *ui_action)
{
    if (!m_actions.contains(ui_action))
        m_actions.insert(ui_action, unique(ui_action->attributeName(), QLatin1String("Qt::Action")));

    return m_actions.value(ui_action);
}

QString Driver::findOrInsertName(const QString &name)
{
    return unique(name);
}

QString Driver::unique(const QString &instanceName, const QString &className)
{
    QString name;
    bool alreadyUsed = false;

    if (instanceName.size()) {
        int id = 1;
        name = instanceName;
        name.replace(QRegExp(QLatin1String("[^a-zA-Z_0-9]")), QLatin1String("_"));

        bool alreadyUsed = false;
        while (true) {
            if (!m_nameRepository.contains(name))
                break;

            alreadyUsed = true;
            name = instanceName + QString::number(id++);
        }
    } else if (className.size()) {
        name = unique(qtify(className));
    } else {
        name = unique(QLatin1String("var"));
    }

    if (alreadyUsed && className.size()) {
        fprintf(stderr, "Warning: name %s is already used\n", instanceName.toLatin1().data());
    }

    m_nameRepository.insert(name, true);
    return name;
}

QString Driver::rubyClassName(const QString &name)
{
    QString qname = name;

    if (qname.startsWith("Q3") && !qname.startsWith("Qt3::")) {
        qname = QString("Qt3::") + qname.mid(2);
    } else if (qname.startsWith("Q") && !qname.startsWith("Qt::")) {
        qname = QString("Qt::") + qname.mid(1);
    } else if (qname.startsWith("K") && !qname.startsWith("KDE::")) {
        qname = QString("KDE::") + qname.mid(1);
    }

    return qname;
}

QString Driver::qtify(const QString &name)
{
    QString qname = name;

    if (qname.startsWith("Qt3::"))
        qname = qname.mid(strlen("Qt3::"));

    if (qname.startsWith("Qt::"))
        qname = qname.mid(strlen("Qt::"));

    if (qname.startsWith("KDE::"))
        qname = qname.mid(strlen("KDE::"));

    int i=0;
    while (i < qname.length()) {
        if (qname.at(i).toLower() != qname.at(i))
            qname[i] = qname.at(i).toLower();
        else
            break;

        ++i;
    }

    return qname;
}

//QString Driver::headerFileName(const QString &fileName)
//{
//    if (fileName.isEmpty())
//        return headerFileName(QLatin1String("noname"));
//
//    QFileInfo info(fileName);
//    return info.baseName().toUpper() + QLatin1String("_H");
//}

bool Driver::printDependencies(const QString &fileName)
{
    Q_ASSERT(m_option.dependencies == true);

    m_option.inputFile = fileName;

    Uic tool(this);
    return tool.printDependencies();
}

bool Driver::uic(const QString &fileName, DomUI *ui, QTextStream *out)
{
    m_option.inputFile = fileName;

    QTextStream *oldOutput = m_output;

    m_output = out != 0 ? out : &m_stdout;

    Uic tool(this);
    bool rtn = tool.write(ui);

    m_output = oldOutput;

    return rtn;
}

bool Driver::uic(const QString &fileName, QTextStream *out)
{
    QFile f;
    if (fileName.isEmpty())
        f.open(stdin, QIODevice::ReadOnly);
    else {
        f.setFileName(fileName);
        if (!f.open(QIODevice::ReadOnly))
            return false;
    }

    m_option.inputFile = fileName;

    QTextStream *oldOutput = m_output;
    bool deleteOutput = false;

    if (out) {
        m_output = out;
    } else {
        m_output = new QTextStream(stdout, QIODevice::WriteOnly);
        deleteOutput = true;
    }

    Uic tool(this);
    bool rtn = tool.write(&f);
    f.close();

    if (deleteOutput)
        delete m_output;

    m_output = oldOutput;

    return rtn;
}

void Driver::reset()
{
    Q_ASSERT( m_output == 0 );

    m_option = Option();
    m_output = 0;
    m_problems.clear();

    QStringList m_problems;

    m_widgets.clear();
    m_spacers.clear();
    m_layouts.clear();
    m_actionGroups.clear();
    m_actions.clear();
    m_nameRepository.clear();
    m_pixmaps.clear();
}

void Driver::insertPixmap(const QString &pixmap)
{
    m_pixmaps.insert(pixmap, true);
}

bool Driver::containsPixmap(const QString &pixmap) const
{
    return m_pixmaps.contains(pixmap);
}

DomWidget *Driver::widgetByName(const QString &name) const
{
    return m_widgets.key(name);
}

DomSpacer *Driver::spacerByName(const QString &name) const
{
    return m_spacers.key(name);
}

DomLayout *Driver::layoutByName(const QString &name) const
{
    return m_layouts.key(name);
}

DomActionGroup *Driver::actionGroupByName(const QString &name) const
{
    return m_actionGroups.key(name);
}

DomAction *Driver::actionByName(const QString &name) const
{
    return m_actions.key(name);
}
