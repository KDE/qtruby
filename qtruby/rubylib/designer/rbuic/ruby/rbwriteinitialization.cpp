/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech ASA. All rights reserved.
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

#include "rbwriteinitialization.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"
#include "databaseinfo.h"
#include "globaldefs.h"

#include <QTextStream>
#include <QtDebug>

#include <limits.h>

namespace Ruby {

WriteInitialization::WriteInitialization(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option()),
      m_defaultMargin(INT_MIN), m_defaultSpacing(INT_MIN),
      delayedOut(&m_delayedInitialization, QIODevice::WriteOnly),
      refreshOut(&m_refreshInitialization, QIODevice::WriteOnly),
      actionOut(&m_delayedActionInitialization, QIODevice::WriteOnly),
      resizeOut(&m_delayedResize, QIODevice::WriteOnly)
{
    this->uic = uic;
}

void WriteInitialization::acceptUI(DomUI *node)
{
    m_registeredImages.clear();
    m_actionGroupChain.push(0);
    m_widgetChain.push(0);
    m_layoutChain.push(0);

    acceptLayoutDefault(node->elementLayoutDefault());
    acceptLayoutFunction(node->elementLayoutFunction());

    if (node->elementCustomWidgets())
        TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());

    if (node->elementImages())
        TreeWalker::acceptImages(node->elementImages());

//    if (option.generateImplemetation)
//        output << "#include <" << driver->headerFileName() << ">\n\n";

    m_stdsetdef = true;
    if (node->hasAttributeStdSetDef())
        m_stdsetdef = node->attributeStdSetDef();

    QString className = node->elementClass() + option.postfix;
    m_generatedClass = className;

    QString varName = driver->findOrInsertWidget(node->elementWidget());
    m_registeredWidgets.insert(varName, node->elementWidget()); // register the main widget

	varName = varName.mid(0, 1).toLower() + varName.mid(1);

    QString widgetClassName = node->elementWidget()->attributeClass();

    output << option.indent << "def " << "setupUi(" << varName << ")\n";

    QStringList connections = uic->databaseInfo()->connections();
    for (int i=0; i<connections.size(); ++i) {
        QString connection = connections.at(i);

        if (connection == QLatin1String("(default)"))
            continue;

        QString varConn = QLatin1String("@") + connection + QLatin1String("Connection");
        output << option.indent << varConn << " = Qt::SqlDatabase.database(" << fixString(connection, option.indent) << ")\n";
    }

    acceptWidget(node->elementWidget());

    for (int i=0; i<m_buddies.size(); ++i) {
        const Buddy &b = m_buddies.at(i);

        if (!m_registeredWidgets.contains(b.objName)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", b.objName.toLatin1().data());
            continue;
        } else if (!m_registeredWidgets.contains(b.buddy)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", b.buddy.toLatin1().data());
            continue;
        }

        output << option.indent << "@" << b.objName << ".setBuddy(@" << b.buddy << ")\n";
    }

    if (node->elementTabStops())
        acceptTabStops(node->elementTabStops());

    if (m_delayedActionInitialization.size())
        output << "\n" << m_delayedActionInitialization;

    output << "\n" << option.indent << "retranslateUi(" << varName << ");\n";

    if (!m_delayedResize.isEmpty())
        output << "\n" << m_delayedResize << "\n";

    if (node->elementConnections())
        acceptConnections(node->elementConnections());

    if (!m_delayedInitialization.isEmpty())
        output << "\n" << m_delayedInitialization << "\n";

    if (option.autoConnection)
        output << "\n" << option.indent << "Qt::MetaObject.connectSlotsByName(" << varName << ")\n";

    output << option.indent << "end # setupUi\n\n";

//    if (m_delayedActionInitialization.isEmpty()) {
//        m_refreshInitialization += option.indent + QLatin1String("Q_UNUSED(") + varName + QLatin1String(");\n");
//    }

    output << option.indent << "def " << "retranslateUi(" << varName << ")\n"
           << m_refreshInitialization
           << option.indent << "end # retranslateUi\n\n";

    m_layoutChain.pop();
    m_widgetChain.pop();
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptWidget(DomWidget *node)
{
    bool isTopLevel = m_widgetChain.count() == 1;
    QString className = node->attributeClass();
    QString varName = driver->findOrInsertWidget(node);
    m_registeredWidgets.insert(varName, node); // register the current widget

	varName = varName.mid(0, 1).toLower() + varName.mid(1);

	if (!isTopLevel) {
		varName.prepend("@");
	}

    QString parentWidget, parentClass;
    if (m_widgetChain.top()) {
        parentWidget = driver->findOrInsertWidget(m_widgetChain.top());
		parentWidget = parentWidget.mid(0, 1).toLower() + parentWidget.mid(1);
		if (m_widgetChain.count() != 2) {
			parentWidget.prepend("@");
		}
        parentClass = m_widgetChain.top()->attributeClass();
    }

    QString savedParentWidget = parentWidget;

    if (uic->isContainer(parentClass) || uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3ToolBar")))
        parentWidget.clear();

    if (m_widgetChain.size() != 1) {
        output << option.indent << varName << " = " << driver->rubyClassName(uic->customWidgetsInfo()->realClassName(className)) << ".new(" << parentWidget << ")\n";
	}

    parentWidget = savedParentWidget;

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QComboBox"))) {
        initializeComboBox(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QListWidget"))) {
        initializeListWidget(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QTreeWidget"))) {
        initializeTreeWidget(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QTableWidget"))) {
        initializeTableWidget(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListBox"))) {
        initializeQ3ListBox(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListView"))) {
        initializeQ3ListView(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3IconView"))) {
        initializeQ3IconView(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3Table"))) {
        initializeQ3Table(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3DataTable"))) {
        initializeQ3SqlDataTable(node);
    } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("Q3DataBrowser"))) {
        initializeQ3SqlDataBrowser(node);
    }

    if (uic->isButton(className)) {
        QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());
        if (DomProperty *prop = attributes.value(QLatin1String("buttonGroup"))) {
            QString groupName = toString(prop->elementString());
            if (!m_buttonGroups.contains(groupName)) {
                m_buttonGroups.insert(groupName, driver->findOrInsertName(groupName));
                QString g = QString("@") + m_buttonGroups.value(groupName);
                output << option.indent << g << " = Qt::ButtonGroup.new(" << m_generatedClass << ")\n";
            }

            QString g = QString("@") + m_buttonGroups.value(groupName);
            output << option.indent << g << ".addButton(" << varName << ")\n";
        }
    }

    writeProperties(varName, className, node->elementProperty());

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QMenu")) && parentWidget.size()) {
        initializeMenu(node, parentWidget);
    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.push(0);

    m_widgetChain.push(node);
    m_layoutChain.push(0);
    TreeWalker::acceptWidget(node);
    m_layoutChain.pop();
    m_widgetChain.pop();

    QHash<QString, DomProperty*> attributes = propertyMap(node->elementAttribute());

    QString title = QLatin1String("Page");
    if (DomProperty *ptitle = attributes.value(QLatin1String("title"))) {
        title = toString(ptitle->elementString());
    }

    QString label = QLatin1String("Page");
    if (DomProperty *plabel = attributes.value(QLatin1String("label"))) {
        label = toString(plabel->elementString());
    }

    int id = -1;
    if (DomProperty *pid = attributes.value(QLatin1String("id"))) {
        id = pid->elementNumber();
    }

    if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QMainWindow"))
            || uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3MainWindow"))) {

        if (uic->customWidgetsInfo()->extends(className, QLatin1String("QMenuBar"))) {
            if (!uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3MainWindow")))
                output << option.indent << parentWidget << ".setMenuBar(" << varName <<")\n";
        } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QToolBar"))) {
            QString area;
            if (DomProperty *pstyle = attributes.value(QLatin1String("toolBarArea"))) {
                area += QLatin1String("");
                area += QString::number(pstyle->elementNumber());
                area += ", ";
            }

            output << option.indent << parentWidget << ".addToolBar(" << area << varName << ")\n";
        } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QDockWidget"))) {
            QString area;
            if (DomProperty *pstyle = attributes.value(QLatin1String("dockWidgetArea"))) {
                area += QLatin1String("");
                area += QString::number(pstyle->elementNumber());
                area += ", ";
            }

            output << option.indent << parentWidget << ".addDockWidget(" << area << varName << ")\n";
        } else if (uic->customWidgetsInfo()->extends(className, QLatin1String("QStatusBar"))) {
            output << option.indent << parentWidget << ".setStatusBar(" << varName << ")\n";
        } else if (className == QLatin1String("QWidget")) {
            output << option.indent << parentWidget << ".setCentralWidget(" << varName << ")\n";
        }
    }

    if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QStackedWidget"))) {
        output << option.indent << parentWidget << ".addWidget(" << varName << ")\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QToolBar"))) {
        output << option.indent << parentWidget << ".addWidget(" << varName << ")\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3WidgetStack"))) {
        output << option.indent << parentWidget << ".addWidget(" << varName << ", " << id << ")\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QDockWidget"))) {
        output << option.indent << parentWidget << ".setWidget(" << varName << ")\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QSplitter"))) {
        output << option.indent << parentWidget << ".addWidget(" << varName << ")\n";
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QToolBox"))) {
        QString icon;
        if (DomProperty *picon = attributes.value(QLatin1String("icon"))) {
            icon += QLatin1String(", ") + pixCall(picon);
        }

        output << option.indent << parentWidget << ".addItem(" << varName << icon << ", " << trCall(label) << ")\n";

        refreshOut << option.indent << parentWidget << ".setItemText("
                   << parentWidget << ".indexOf(" << varName << "), " << trCall(label) << ")\n";

        if (DomProperty *ptoolTip = attributes.value(QLatin1String("toolTip"))) {
            refreshOut << option.indent << parentWidget << ".setItemToolTip("
                       << parentWidget << ".indexOf(" << varName << "), " << trCall(ptoolTip->elementString()) << ")\n";
        }
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("QTabWidget"))) {
        QString icon;
        if (DomProperty *picon = attributes.value(QLatin1String("icon"))) {
            icon += QLatin1String(", ") + pixCall(picon);
        }

        output << option.indent << parentWidget << ".addTab(" << varName << icon << ", " << trCall(title) << ")\n";

        refreshOut << option.indent << parentWidget << ".setTabText("
                   << parentWidget << ".indexOf(" << varName << "), " << trCall(title) << ")\n";

        if (DomProperty *ptoolTip = attributes.value(QLatin1String("toolTip"))) {
            refreshOut << option.indent << parentWidget << ".setTabToolTip("
                       << parentWidget << ".indexOf(" << varName << "), " << trCall(ptoolTip->elementString()) << ")\n";
        }
    } else if (uic->customWidgetsInfo()->extends(parentClass, QLatin1String("Q3Wizard"))) {
        output << option.indent << parentWidget << ".addPage(" << varName << ", " << trCall(title) << ")\n";

        refreshOut << option.indent << parentWidget << ".setTitle("
                   << varName << ", " << trCall(title) << ")\n";

    }

    if (node->elementLayout().isEmpty())
        m_layoutChain.pop();
}

void WriteInitialization::acceptLayout(DomLayout *node)
{
    QString className = node->attributeClass();
    QString varName = toRubyIdentifier(driver->findOrInsertLayout(node));

    QHash<QString, DomProperty*> properties = propertyMap(node->elementProperty());

    bool isGroupBox = false;

    if (m_widgetChain.top()) {
        QString parentWidget = m_widgetChain.top()->attributeClass();

        if (!m_layoutChain.top() && (uic->customWidgetsInfo()->extends(parentWidget, QLatin1String("Q3GroupBox"))
                        || uic->customWidgetsInfo()->extends(parentWidget, QLatin1String("Q3ButtonGroup")))) {
            QString parent = toRubyIdentifier(driver->findOrInsertWidget(m_widgetChain.top()));

            isGroupBox = true;

            // special case for group box

            int margin = m_defaultMargin;
            int spacing = m_defaultSpacing;

            if (properties.contains(QLatin1String("margin")))
                margin = properties.value(QLatin1String("margin"))->elementNumber();

            if (properties.contains(QLatin1String("spacing")))
                spacing = properties.value(QLatin1String("spacing"))->elementNumber();

            output << option.indent << parent << ".setColumnLayout(0, Qt::Vertical)\n";

            if (spacing != INT_MIN) {
                QString value = QString::number(spacing);
                if (!m_spacingFunction.isEmpty() && spacing == m_defaultSpacing)
                    value = m_spacingFunction + QLatin1String("()");

                output << option.indent << parent << ".layout().setSpacing(" << value << ")\n";
            }

            if (margin != INT_MIN) {
                QString value = QString::number(margin);
                if (!m_marginFunction.isEmpty() && margin == m_defaultMargin)
                    value = m_marginFunction + QLatin1String("()");

                output << option.indent << parent << ".layout().setMargin(" << value << ")\n";
            }
        }
    }

    output << option.indent << varName << " = " << driver->rubyClassName(className) << ".new(";

	QString parentWidget = driver->findOrInsertWidget(m_widgetChain.top());
	parentWidget = parentWidget.mid(0, 1).toLower() + parentWidget.mid(1);
	if (m_widgetChain.count() != 2) {
		parentWidget.prepend("@");
	}

    if (isGroupBox) {
        output << parentWidget << ".layout()";
    } else if (!m_layoutChain.top()) {
        output << parentWidget;
    }

    output << ")\n";

    QList<DomProperty*> layoutProperties = node->elementProperty();

    if (isGroupBox) {
        output << option.indent << varName << ".alignment = Qt::AlignTop\n";
    } else {
        int margin = m_defaultMargin;
        int spacing = m_defaultSpacing;

        if (properties.contains(QLatin1String("margin"))) {
            DomProperty *p = properties.value(QLatin1String("margin"));
            Q_ASSERT(p != 0);

            margin = properties.value(QLatin1String("margin"))->elementNumber();
            layoutProperties.removeAt(layoutProperties.indexOf(p));
        }

        if (properties.contains(QLatin1String("spacing"))) {
            DomProperty *p = properties.value(QLatin1String("spacing"));
            Q_ASSERT(p != 0);

            spacing = properties.value(QLatin1String("spacing"))->elementNumber();
            layoutProperties.removeAt(layoutProperties.indexOf(p));
        }

        if (spacing != INT_MIN) {
            QString value = QString::number(spacing);
            if (!m_spacingFunction.isEmpty() && spacing == m_defaultSpacing)
                value = m_spacingFunction + QLatin1String("()");

            output << option.indent << varName << ".spacing = " << value << "\n";
        }

        if (margin != INT_MIN) {
            QString value = QString::number(margin);
            if (!m_marginFunction.isEmpty() && margin == m_defaultMargin)
                value = m_marginFunction + QLatin1String("()");

            output << option.indent << varName << ".margin = " << margin << "\n";
        }
    }

    writeProperties(varName, className, layoutProperties);

    m_layoutChain.push(node);
    TreeWalker::acceptLayout(node);
    m_layoutChain.pop();
}

void WriteInitialization::acceptSpacer(DomSpacer *node)
{
    QHash<QString, DomProperty *> properties = propertyMap(node->elementProperty());
    QString varName = toRubyIdentifier(driver->findOrInsertSpacer(node));

    DomSize *sizeHint = properties.contains(QLatin1String("sizeHint"))
        ? properties.value(QLatin1String("sizeHint"))->elementSize() : 0;

    QString sizeType = properties.contains(QLatin1String("sizeType"))
        ? properties.value(QLatin1String("sizeType"))->elementEnum()
        : QString::fromLatin1("Expanding");

    QString orientation = properties.contains(QLatin1String("orientation"))
        ? properties.value(QLatin1String("orientation"))->elementEnum() : QString();

    bool isVspacer = orientation == QLatin1String("Qt::Vertical")
        || orientation == QLatin1String("Vertical");

    output << option.indent << varName << " = Qt::SpacerItem.new(";

    if (sizeHint)
        output << sizeHint->elementWidth() << ", " << sizeHint->elementHeight() << ", ";

    if (sizeType.startsWith(QLatin1String("QSizePolicy::"))) {
        sizeType.replace("QSizePolicy::", "Qt::SizePolicy::");
	} else {
        sizeType.prepend(QLatin1String("Qt::SizePolicy::"));
    }

    if (isVspacer)
        output << "Qt::SizePolicy::Minimum, " << sizeType << ")\n";
    else
        output << sizeType << ", Qt::SizePolicy::Minimum)\n";

    TreeWalker::acceptSpacer(node);
}

void WriteInitialization::acceptLayoutItem(DomLayoutItem *node)
{
    TreeWalker::acceptLayoutItem(node);

    DomLayout *layout = m_layoutChain.top();

    if (!layout)
        return;

    QString varName = toRubyIdentifier(driver->findOrInsertLayoutItem(node));
    QString layoutName = toRubyIdentifier(driver->findOrInsertLayout(layout));

    QString opt;
    if (layout->attributeClass() == QLatin1String("QGridLayout")) {
        int row = node->attributeRow();
        int col = node->attributeColumn();

        int rowSpan = 1;
        if (node->hasAttributeRowSpan())
            rowSpan = node->attributeRowSpan();

        int colSpan = 1;
        if (node->hasAttributeColSpan())
            colSpan = node->attributeColSpan();

        opt = QString::fromLatin1(", %1, %2, %3, %4").arg(row).arg(col).arg(rowSpan).arg(colSpan);
    }

    QString method = QLatin1String("addItem");
    switch (node->kind()) {
        case DomLayoutItem::Widget:
            method = QLatin1String("addWidget");
            break;
        case DomLayoutItem::Layout:
            method = QLatin1String("addLayout");
            break;
        case DomLayoutItem::Spacer:
            method = QLatin1String("addItem");
            break;
        case DomLayoutItem::Unknown:
            Q_ASSERT( 0 );
            break;
    }

    output << "\n" << option.indent << layoutName << "." << method << "(" << varName << opt << ")\n\n";
}

void WriteInitialization::acceptActionGroup(DomActionGroup *node)
{
    QString actionName = toRubyIdentifier(driver->findOrInsertActionGroup(node));

    QString varName = driver->findOrInsertWidget(m_widgetChain.top());
	varName = varName.mid(0, 1).toLower() + varName.mid(1);
	if (m_widgetChain.count() > 2) {
		varName.prepend("@");
	}

    if (m_actionGroupChain.top())
        varName = driver->findOrInsertActionGroup(m_actionGroupChain.top());

    output << option.indent << actionName << " = Qt:ActionGroup.new(" << varName << ")\n";
    writeProperties(actionName, QLatin1String("QActionGroup"), node->elementProperty());

    m_actionGroupChain.push(node);
    TreeWalker::acceptActionGroup(node);
    m_actionGroupChain.pop();
}

void WriteInitialization::acceptAction(DomAction *node)
{
    if (node->hasAttributeMenu())
        return;

    QString actionName = toRubyIdentifier(driver->findOrInsertAction(node));

    m_registeredActions.insert(actionName, node);
    QString varName = driver->findOrInsertWidget(m_widgetChain.top());
	varName = varName.mid(0, 1).toLower() + varName.mid(1);
	if (m_widgetChain.count() > 2) {
		varName.prepend("@");
	}

    if (m_actionGroupChain.top()) {
        varName = toRubyIdentifier(driver->findOrInsertActionGroup(m_actionGroupChain.top()));
    }

    output << option.indent << actionName << " = Qt::Action.new(" << varName << ")\n";
    writeProperties(actionName, QLatin1String("QAction"), node->elementProperty());
}

void WriteInitialization::acceptActionRef(DomActionRef *node)
{
    QString actionName = toRubyIdentifier(node->attributeName());
    bool isSeparator = actionName == QLatin1String("@separator");
    bool isMenu = false;

    QString varName = driver->findOrInsertWidget(m_widgetChain.top());
	varName = varName.mid(0, 1).toLower() + varName.mid(1);
	if (m_widgetChain.count() > 2) {
		varName.prepend("@");
	}

    if (actionName.isEmpty() || !m_widgetChain.top()) {
        return;
    } else if (driver->actionGroupByName(actionName)) {
        return;
    } else if (DomWidget *w = driver->widgetByName(actionName)) {
        isMenu = uic->isMenu(w->attributeClass());
        bool inQ3ToolBar = uic->customWidgetsInfo()->extends(m_widgetChain.top()->attributeClass(), QLatin1String("Q3ToolBar"));
        if (!isMenu && inQ3ToolBar) {
            actionOut << option.indent << actionName << ".setParent(" << varName << ")\n";
            return;
        }
    } else if (!(driver->actionByName(actionName) || isSeparator)) {
        fprintf(stderr, "Warning: action `%s' not declared\n", actionName.toLatin1().data());
        return;
    }

    if (m_widgetChain.top() && isSeparator) {
        // separator is always reserved!
        actionOut << option.indent << varName << ".addSeparator()\n";
        return;
    }

    if (isMenu)
        actionName += QLatin1String(".menuAction()");

    actionOut << option.indent << varName << ".addAction(" << actionName << ")\n";
}

void WriteInitialization::writeProperties(const QString &varName,
                                          const QString &className,
                                          const QList<DomProperty*> &lst)
{
    bool isTopLevel = m_widgetChain.count() == 1;

    if (uic->customWidgetsInfo()->extends(className, QLatin1String("QAxWidget"))) {
        QHash<QString, DomProperty*> properties = propertyMap(lst);
        if (properties.contains(QLatin1String("control"))) {
            DomProperty *p = properties.value(QLatin1String("control"));
            output << option.indent << varName << ".setControl("
                   << fixString(toString(p->elementString()), option.indent) << ")\n";
        }
    }

    DomWidget *buttonGroupWidget = findWidget(QLatin1String("Q3ButtonGroup"));

	if (varName.startsWith("@")) {
    	output << option.indent << varName << ".setObjectName(" << fixString(varName.mid(1), option.indent) << ")\n";
 	} else {
   		output << option.indent << varName << ".setObjectName(" << fixString(varName.mid(0,1).toLower() + varName.mid(1), option.indent) << ")\n";
	}

    for (int i=0; i<lst.size(); ++i) {
        DomProperty *p = lst.at(i);
        QString propertyName = p->attributeName();
        QString propertyValue;

        // special case for the property `geometry'
        if (isTopLevel && propertyName == QLatin1String("geometry") && p->elementRect()) {
            DomRect *r = p->elementRect();
            int w = r->elementWidth();
            int h = r->elementHeight();
			QString tempName = driver->unique(QLatin1String("size"));
			resizeOut << option.indent << tempName << " = Qt::Size.new(" << w << ", " << h << ")\n"
                      << option.indent << tempName << " = " << tempName << ".expandedTo("
                      << varName << ".minimumSizeHint())\n"
                      << option.indent << varName << ".resize(" << tempName << ")\n";                
            continue;
        } else if (propertyName == QLatin1String("buttonGroupId") && buttonGroupWidget) { // Q3ButtonGroup support
            output << option.indent << driver->findOrInsertWidget(buttonGroupWidget) << ".insert("
                   << varName << ", " << p->elementNumber() << ")\n";
            continue;
        } else if (propertyName == QLatin1String("currentRow") // QListWidget::currentRow
                    && uic->customWidgetsInfo()->extends(className, QLatin1String("QListWidget"))) {
            delayedOut << option.indent << varName << ".setCurrentRow("
                       << p->elementNumber() << ")\n";
            continue;
        } else if (propertyName == QLatin1String("currentIndex") // set currentIndex later
                    && (uic->customWidgetsInfo()->extends(className, QLatin1String("QComboBox"))
                    || uic->customWidgetsInfo()->extends(className, QLatin1String("QStackedWidget"))
                    || uic->customWidgetsInfo()->extends(className, QLatin1String("QTabWidget"))
                    || uic->customWidgetsInfo()->extends(className, QLatin1String("QToolBox")))) {
            delayedOut << option.indent << varName << ".setCurrentIndex("
                       << p->elementNumber() << ")\n";
            continue;
        } else if (propertyName == QLatin1String("control") // ActiveQt support
                    && uic->customWidgetsInfo()->extends(className, QLatin1String("QAxWidget"))) {
            // already done ;)
            continue;
        } else if (propertyName == QLatin1String("database")
                    && p->elementStringList()) {
            // Sql support
            continue;
        } else if (propertyName == QLatin1String("frameworkCode")
                    && p->kind() == DomProperty::Bool) {
            // Sql support
            continue;
        } else if (propertyName == QLatin1String("orientation")
                    && uic->customWidgetsInfo()->extends(className, QLatin1String("Line"))) {
            // Line support
            QString shape = QLatin1String("Qt::Frame::HLine");
            if (p->elementEnum() == QLatin1String("Qt::Vertical"))
                shape = QLatin1String("Qt::Frame::VLine");

            output << option.indent << varName << ".setFrameShape(" << shape << ")\n";
            output << option.indent << varName << ".setFrameShadow(Qt::Frame::Sunken)\n";
            continue;
        }

        bool stdset = m_stdsetdef;
        if (p->hasAttributeStdset())
            stdset = p->attributeStdset();

        QString setFunction;

        if (stdset) {
            setFunction = QLatin1String(".set")
                + propertyName.left(1).toUpper()
                + propertyName.mid(1)
                + QLatin1String("(");
        } else {
            setFunction = QLatin1String(".setProperty(\"")
                + propertyName
                + QLatin1String("\", Qt::Variant.new(");
        }

        switch (p->kind()) {
        case DomProperty::Bool: {
            propertyValue = p->elementBool();
            break;
        }
        case DomProperty::Color: {
            DomColor *c = p->elementColor();
            propertyValue = QString::fromLatin1("Qt::Color.new(%1, %2, %3)")
                  .arg(c->elementRed())
                  .arg(c->elementGreen())
                  .arg(c->elementBlue()); }
            break;
        case DomProperty::Cstring:
            if (propertyName == QLatin1String("buddy") && uic->customWidgetsInfo()->extends(className, QLatin1String("QLabel"))) {
                m_buddies.append(Buddy(varName, p->elementCstring()));
            } else {
                if (stdset)
                    propertyValue = fixString(p->elementCstring(), option.indent);
                else
                    propertyValue = QLatin1String("Qt::ByteArray.new(") + fixString(p->elementCstring(), option.indent) + QLatin1String(")");
            }
            break;
        case DomProperty::Cursor:
            propertyValue = QString::fromLatin1("Qt::Cursor.new(%1)")
                            .arg(p->elementCursor());
            break;
        case DomProperty::Enum:
            propertyValue = p->elementEnum();
            if (!propertyValue.contains(QLatin1String("::")))
                propertyValue.prepend(className + QLatin1String(QLatin1String("::")));
			propertyValue = driver->rubyClassName(propertyValue);
            break;
        case DomProperty::Set:
            propertyValue = p->elementSet();
            if (!propertyValue.contains(QLatin1String("::")))
                propertyValue.prepend(className + QLatin1String(QLatin1String("::")));
			propertyValue = driver->rubyClassName(propertyValue);
			propertyValue.replace("|", "|");
            break;
        case DomProperty::Font: {
            DomFont *f = p->elementFont();
            QString fontName = QString("@") + driver->unique(QLatin1String("font"));
            output << option.indent << fontName << " = Qt::Font.new\n";
            if (f->hasElementFamily() && !f->elementFamily().isEmpty()) {
                output << option.indent << fontName << ".setFamily(" << fixString(f->elementFamily(), option.indent)
                    << ")\n";
            }
            if (f->hasElementPointSize() && f->elementPointSize() > 0) {
                output << option.indent << fontName << ".setPointSize(" << f->elementPointSize()
                    << ")\n";
            }
            if (f->hasElementBold()) {
                output << option.indent << fontName << ".setBold("
                    << (f->elementBold() ? "true" : "false") << ")\n";
            }
            if (f->hasElementItalic()) {
                output << option.indent << fontName << ".setItalic("
                    <<  (f->elementItalic() ? "true" : "false") << ")\n";
            }
            if (f->hasElementUnderline()) {
                output << option.indent << fontName << ".setUnderline("
                    << (f->elementUnderline() ? "true" : "false") << ")\n";
            }
            if (f->hasElementWeight() && f->elementWeight() > 0) {
                output << option.indent << fontName << ".setWeight("
                    << f->elementWeight() << ")" << endl;
            }
            if (f->hasElementStrikeOut()) {
                output << option.indent << fontName << ".setStrikeOut("
                    << (f->elementStrikeOut() ? "true" : "false") << ")\n";
            }
            if (f->hasElementKerning()) {
                output << option.indent << fontName << ".setKerning("
                    << (f->elementKerning() ? "true" : "false") << ")\n";
            }
            if (f->hasElementAntialiasing()) {
                output << option.indent << fontName << ".setStyleStrategy("
                    << (f->elementAntialiasing() ? "Qt::Font::PreferDefault" : "Qt::Font::NoAntialias") << ")\n";
            }
            propertyValue = fontName;
            break;
        }
        case DomProperty::IconSet:
            propertyValue = pixCall(p);
            break;

        case DomProperty::Pixmap:
            propertyValue = pixCall(p);
            break;

        case DomProperty::Palette: {
            DomPalette *pal = p->elementPalette();
            QString paletteName = QString("@") + driver->unique(QLatin1String("palette"));
            output << option.indent << paletteName << " = Qt::Palette.new\n";

            writeColorGroup(pal->elementActive(), QLatin1String("Qt::Palette::Active"), paletteName);
            writeColorGroup(pal->elementInactive(), QLatin1String("Qt::Palette::Inactive"), paletteName);
            writeColorGroup(pal->elementDisabled(), QLatin1String("Qt::Palette::Disabled"), paletteName);

            propertyValue = paletteName;
            break;
        }
        case DomProperty::Point: {
            DomPoint *po = p->elementPoint();
            propertyValue = QString::fromLatin1("Qt::Point.new(%1, %2)")
                            .arg(po->elementX()).arg(po->elementY());
            break;
        }
        case DomProperty::PointF: {
            DomPointF *pof = p->elementPointF();
            propertyValue = QString::fromLatin1("Qt::PointF.new(%1, %2)")
                            .arg(pof->elementX()).arg(pof->elementY());
            break;
        }
        case DomProperty::Rect: {
            DomRect *r = p->elementRect();
            propertyValue = QString::fromLatin1("Qt::Rect.new(%1, %2, %3, %4)")
                            .arg(r->elementX()).arg(r->elementY())
                            .arg(r->elementWidth()).arg(r->elementHeight());
            break;
        }
        case DomProperty::RectF: {
            DomRectF *rf = p->elementRectF();
            propertyValue = QString::fromLatin1("Qt::RectF.new(%1, %2, %3, %4)")
                            .arg(rf->elementX()).arg(rf->elementY())
                            .arg(rf->elementWidth()).arg(rf->elementHeight());
            break;
        }
        case DomProperty::SizePolicy: {
            DomSizePolicy *sp = p->elementSizePolicy();
            QString spName = QString("@") + driver->unique(QLatin1String("sizePolicy"));
            output << option.indent << spName << QString::fromLatin1(
                " = Qt::SizePolicy.new(%1, %2)\n")
                            .arg(sp->elementHSizeType())
                            .arg(sp->elementVSizeType());
            output << option.indent << spName << ".setHorizontalStretch("
                << sp->elementHorStretch() << ")\n";
            output << option.indent << spName << ".setVerticalStretch("
                << sp->elementVerStretch() << ")\n";
            output << option.indent << spName << QString::fromLatin1(
                ".setHeightForWidth(%1.sizePolicy().hasHeightForWidth())\n")
                .arg(varName);

            propertyValue = spName;
            break;
        }
        case DomProperty::Size: {
             DomSize *s = p->elementSize();
              propertyValue = QString::fromLatin1("Qt::Size.new(%1, %2)")
                             .arg(s->elementWidth()).arg(s->elementHeight());
            break;
        }
        case DomProperty::SizeF: {
            DomSizeF *sf = p->elementSizeF();
             propertyValue = QString::fromLatin1("Qt::SizeF.new(%1, %2)")
                            .arg(sf->elementWidth()).arg(sf->elementHeight());
            break;
        }
        case DomProperty::String: {
            if (propertyName == QLatin1String("objectName")) {
                QString v = p->elementString()->text();
				if (	(varName.startsWith("@") && v == varName.mid(1))
						|| v.mid(0,1).toLower() + v.mid(1) == varName )
					break;

                // ### qWarning("Deprecated: the property `objectName' is different from the variable name");
            }

            if (p->elementString()->hasAttributeNotr()
                    && toBool(p->elementString()->attributeNotr())) {
                propertyValue = fixString(p->elementString()->text(), option.indent);
            } else {
                propertyValue = trCall(p->elementString());
            }
            break;
        }
        case DomProperty::Number:
            propertyValue = QString::number(p->elementNumber());
            break;
        case DomProperty::LongLong:
            propertyValue = QString::number(p->elementLongLong());
            break;
        case DomProperty::Float:
            propertyValue = QString::number(p->elementFloat());
            break;
        case DomProperty::Double:
            propertyValue = QString::number(p->elementDouble());
            break;
        case DomProperty::Char: {
            DomChar *c = p->elementChar();
            propertyValue = QString::fromLatin1("Qt::Char.new(%1)")
                            .arg(c->elementUnicode());
            break;
        }
        case DomProperty::Date: {
            DomDate *d = p->elementDate();
            propertyValue = QString::fromLatin1("Qt::Date.new(%1, %2, %3)")
                            .arg(d->elementYear())
                            .arg(d->elementMonth())
                            .arg(d->elementDay());
            break;
        }
        case DomProperty::Time: {
            DomTime *t = p->elementTime();
            propertyValue = QString::fromLatin1("Qt::Time.new(%1, %2, %3)")
                            .arg(t->elementHour())
                            .arg(t->elementMinute())
                            .arg(t->elementSecond());
            break;
        }
        case DomProperty::DateTime: {
            DomDateTime *dt = p->elementDateTime();
            propertyValue = QString::fromLatin1("Qt::DateTime.new(Qt::Date.new(%1, %2, %3), Qt::Time.new(%4, %5, %6))")
                            .arg(dt->elementYear())
                            .arg(dt->elementMonth())
                            .arg(dt->elementDay())
                            .arg(dt->elementHour())
                            .arg(dt->elementMinute())
                            .arg(dt->elementSecond());
            break;
        }
        case DomProperty::StringList:
            propertyValue = QLatin1String("[]");
            if (p->elementStringList()->elementString().size()) {
                QStringList lst = p->elementStringList()->elementString();
                for (int i=0; i<lst.size(); ++i) {
                    propertyValue += QLatin1String(" << ") + fixString(lst.at(i), option.indent);
                }
            }
            break;

        case DomProperty::Url: {
            DomUrl* u = p->elementUrl();
            propertyValue = QString::fromLatin1("Qt::Url.new(%1)")
                            .arg(fixString(u->elementString()->text(), option.indent));
            break;
        }
        case DomProperty::Unknown:
            break;
        }

        if (propertyValue.size()) {
            QTextStream *o = &output;

            if (p->kind() == DomProperty::String
                    && (!p->elementString()->hasAttributeNotr() || !toBool(p->elementString()->attributeNotr())))
                o = &refreshOut;

            (*o) << option.indent << varName << setFunction << propertyValue;
            if (!stdset)
                (*o) << ")";
            (*o) << ")\n";
        }
    }
}

QString WriteInitialization::domColor2QString(DomColor *c)
{
    if (c->hasAttributeAlpha())
        return QString::fromLatin1("Qt::Color.new(%1, %2, %3, %4)")
            .arg(c->elementRed())
            .arg(c->elementGreen())
            .arg(c->elementBlue())
            .arg(c->attributeAlpha());
    return QString::fromLatin1("Qt::Color.new(%1, %2, %3)")
        .arg(c->elementRed())
        .arg(c->elementGreen())
        .arg(c->elementBlue());
}

void WriteInitialization::writeColorGroup(DomColorGroup *colorGroup, const QString &group, const QString &paletteName)
{
    if (!colorGroup)
        return;

    // old format
    QList<DomColor*> colors = colorGroup->elementColor();
    for (int i=0; i<colors.size(); ++i) {
        DomColor *color = colors.at(i);

        output << option.indent << paletteName << ".setColor(" << group
            << ", " << i 
            << ", " << domColor2QString(color)
            << ")\n";
    }

    // new format
    QList<DomColorRole *> colorRoles = colorGroup->elementColorRole();
    QListIterator<DomColorRole *> itRole(colorRoles);
    while (itRole.hasNext()) {
        DomColorRole *colorRole = itRole.next();
        if (colorRole->hasAttributeRole()) {
            QString brushName = driver->unique(QLatin1String("brush"));
            writeBrush(colorRole->elementBrush(), brushName);

            output << option.indent << paletteName << ".setBrush(" << group
                << ", " << "Qt::Palette::" << colorRole->attributeRole()
                << ", " << brushName << ")\n";
        }
    }
}

void WriteInitialization::writeBrush(DomBrush *brush, const QString &brushName)
{
    QString style = QLatin1String("SolidPattern");
    if (brush->hasAttributeBrushStyle())
        style = brush->attributeBrushStyle();

    if (style == QLatin1String("LinearGradientPattern") ||
            style == QLatin1String("RadialGradientPattern") ||
            style == QLatin1String("ConicalGradientPattern")) {
        DomGradient *gradient = brush->elementGradient();
        QString gradientType = gradient->attributeType();
        QString gradientName = driver->unique(QLatin1String("gradient"));
        if (gradientType == QLatin1String("LinearGradient")) {
            output << option.indent << gradientName << " = Qt::LinearGradient.new" 
                << "(" << gradient->attributeStartX()
                << ", " << gradient->attributeStartY()
                << ", " << gradient->attributeEndX()
                << ", " << gradient->attributeEndY() << ")\n";
        } else if (gradientType == QLatin1String("RadialGradient")) {
            output << option.indent << gradientName << " = Qt::RadialGradient.new" 
                << "(" << gradient->attributeCentralX()
                << ", " << gradient->attributeCentralY()
                << ", " << gradient->attributeRadius()
                << ", " << gradient->attributeFocalX()
                << ", " << gradient->attributeFocalY() << ")\n";
        } else if (gradientType == QLatin1String("ConicalGradient")) {
            output << option.indent << gradientName << " = Qt::ConicalGradient.new"
                << "(" << gradient->attributeCentralX()
                << ", " << gradient->attributeCentralY()
                << ", " << gradient->attributeAngle() << ");\n";
        }

        output << option.indent << gradientName << ".setSpread(Qt::Gradient::"
            << gradient->attributeSpread() << ")\n";

        if (gradient->hasAttributeCoordinateMode()) {
            output << option.indent << gradientName << ".setCoordinateMode(Qt::Gradient::"
                << gradient->attributeCoordinateMode() << ")\n";
        }

        QList<DomGradientStop *> stops = gradient->elementGradientStop();
        QListIterator<DomGradientStop *> it(stops);
        while (it.hasNext()) {
            DomGradientStop *stop = it.next();
            DomColor *color = stop->elementColor();
            output << option.indent << gradientName << ".setColorAt("
                << stop->attributePosition() << ", "
                << domColor2QString(color) << ")\n";
        }
        output << option.indent << brushName << "= Qt::Brush.new" << "("
            << gradientName << ")\n";
    } else if (style == QLatin1String("TexturePattern")) {
        DomProperty *property = brush->elementTexture();

        output << option.indent << brushName << "= Qt::Brush.new" <<  "("
            << pixCall(property) << ")\n";
    } else {
        DomColor *color = brush->elementColor();
        output << option.indent << brushName << "= Qt::Brush.new" <<  "("
            << domColor2QString(color) << ")\n";

        output << option.indent << brushName << ".setStyle("
            << "Qt::" << style << ")\n";
    }
}

void WriteInitialization::acceptCustomWidget(DomCustomWidget *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::acceptCustomWidgets(DomCustomWidgets *node)
{
    Q_UNUSED(node);
}

void WriteInitialization::acceptTabStops(DomTabStops *tabStops)
{
    QString lastName;

    QStringList l = tabStops->elementTabStop();
    for (int i=0; i<l.size(); ++i) {
        QString name = l.at(i);

        if (!m_registeredWidgets.contains(name)) {
            fprintf(stderr, "'%s' isn't a valid widget\n", name.toLatin1().data());
            continue;
        }

        if (i == 0) {
            lastName = name;
            continue;
        } else if (name.isEmpty() || lastName.isEmpty()) {
            continue;
        }

        output << option.indent << "Qt::Widget::setTabOrder(@" << lastName << ", @" << name << ")\n";

        lastName = name;
    }
}

void WriteInitialization::acceptLayoutDefault(DomLayoutDefault *node)
{
    m_defaultMargin = INT_MIN;
    m_defaultSpacing = INT_MIN;

    if (!node)
        return;

    if (node->hasAttributeMargin())
        m_defaultMargin = node->attributeMargin();

    if (node->hasAttributeSpacing())
        m_defaultSpacing = node->attributeSpacing();
}

void WriteInitialization::acceptLayoutFunction(DomLayoutFunction *node)
{
    m_marginFunction.clear();
    m_spacingFunction.clear();

    if (!node)
        return;

    if (node->hasAttributeMargin())
        m_marginFunction = node->attributeMargin();

    if (node->hasAttributeSpacing())
        m_spacingFunction = node->attributeSpacing();
}

void WriteInitialization::initializeQ3ListBox(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << option.indent << varName << ".clear()\n";

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        if (!(text || pixmap))
            continue;

        refreshOut << option.indent << varName << ".insertItem(";
        if (pixmap) {
            refreshOut << pixCall(pixmap);

            if (text)
                refreshOut << ", ";
        }
        if (text)
            refreshOut << trCall(text->elementString());
        refreshOut << ")\n";
    }
}

void WriteInitialization::initializeQ3IconView(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << option.indent << varName << ".clear()\n";

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        if (!(text || pixmap))
            continue;

        QString itemName = driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << option.indent << itemName << " = Qt::3IconViewItem.new(" << varName << ")\n";

        if (pixmap) {
            refreshOut << option.indent << itemName << ".setPixmap(" << pixCall(pixmap) << ")\n";
        }

        if (text) {
            refreshOut << option.indent << itemName << ".setText(" << trCall(text->elementString()) << ")\n";
        }
    }
}

void WriteInitialization::initializeQ3ListView(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    // columns
    QList<DomColumn*> columns = w->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        DomProperty *clickable = properties.value(QLatin1String("clickable"));
        DomProperty *resizable = properties.value(QLatin1String("resizable"));

        QString txt = trCall(text->elementString());
        output << option.indent << varName << ".addColumn(" << txt << ")\n";
        refreshOut << option.indent << varName << ".header().setLabel(" << i << ", " << txt << ")\n";

        if (pixmap) {
            output << option.indent << varName << ".header().setLabel("
                   << varName << ".header().count() - 1, " << pixCall(pixmap) << ", " << txt << ")\n";
        }

        if (clickable != 0) {
            output << option.indent << varName << ".header().setClickEnabled(" << clickable->elementBool() << ", " << varName << ".header().count() - 1)\n";
        }

        if (resizable != 0) {
            output << option.indent << varName << ".header().setResizeEnabled(" << resizable->elementBool() << ", " << varName << ".header().count() - 1)\n";
        }
    }

    if (w->elementItem().size()) {
        refreshOut << option.indent << varName << ".clear()\n";

        initializeQ3ListViewItems(className, varName, w->elementItem());
    }
}

void WriteInitialization::initializeQ3ListViewItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    if (items.isEmpty())
        return;

    // items
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QString itemName = driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << option.indent << itemName << " = new Qt3::ListViewItem.new(" << varName << ")\n";

        int textCount = 0, pixCount = 0;
        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                refreshOut << option.indent << itemName << ".setText(" << textCount++ << ", "
                           << trCall(p->elementString()) << ")\n";

            if (p->attributeName() == QLatin1String("pixmap"))
                refreshOut << option.indent << itemName << ".setPixmap(" << pixCount++ << ", "
                           << pixCall(p) << ")\n";
        }

        if (item->elementItem().size()) {
            refreshOut << option.indent << itemName << ".setOpen(true)\n";
            initializeQ3ListViewItems(className, itemName, item->elementItem());
        }
    }
}

void WriteInitialization::initializeTreeWidgetItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    if (items.isEmpty())
        return;

    // items
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QString itemName = driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << option.indent << itemName << " = Qt::TreeWidgetItem.new(" << varName << ")\n";

        int textCount = 0;
        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                refreshOut << option.indent << itemName << ".setText(" << textCount++ << ", "
                           << trCall(p->elementString()) << ")\n";

            if (p->attributeName() == QLatin1String("icon") && textCount > 0)
                refreshOut << option.indent << itemName << ".setIcon(" << textCount - 1 << ", "
                           << pixCall(p) << ")\n";
        }

        if (item->elementItem().size())
            initializeTreeWidgetItems(className, itemName, item->elementItem());
    }
}


void WriteInitialization::initializeQ3Table(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    // columns
    QList<DomColumn*> columns = w->elementColumn();

    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));

        refreshOut << option.indent << varName << ".horizontalHeader().setLabel(" << i << ", ";
        if (pixmap) {
            refreshOut << pixCall(pixmap) << ", ";
        }
        refreshOut << trCall(text->elementString()) << ")\n";
    }

    // rows
    QList<DomRow*> rows = w->elementRow();
    for (int i=0; i<rows.size(); ++i) {
        DomRow *row = rows.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(row->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));

        refreshOut << option.indent << varName << ".verticalHeader().setLabel(" << i << ", ";
        if (pixmap) {
            refreshOut << pixCall(pixmap) << ", ";
        }
        refreshOut << trCall(text->elementString()) << ")\n";
    }


    //initializeQ3TableItems(className, varName, w->elementItem());
}

void WriteInitialization::initializeQ3TableItems(const QString &className, const QString &varName, const QList<DomItem *> &items)
{
    Q_UNUSED(className);
    Q_UNUSED(varName);
    Q_UNUSED(items);
}

QString WriteInitialization::pixCall(DomProperty *p) const
{
    Q_ASSERT(p->kind() == DomProperty::IconSet || p->kind() == DomProperty::Pixmap);

    QString type, s;
    if (p->kind() == DomProperty::IconSet) {
        type = QLatin1String("Qt::Icon.new");
        s = p->elementIconSet()->text();
    } else {
        type = QLatin1String("Qt::Pixmap.new");
        s = p->elementPixmap()->text();
    }

    if (s.isEmpty())
        return type + QLatin1String("()");
    else if (findImage(s) != 0)
        return QLatin1String("icon(") + s + QLatin1String("_ID)");

    QString pixFunc = uic->pixmapFunction();

    return type + QLatin1String("(") + fixString(s, option.indent) + QLatin1String(")");
}

void WriteInitialization::initializeComboBox(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << option.indent << varName << ".clear()\n";

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("icon"));
        if (!(text || pixmap))
            continue;

        refreshOut << option.indent << varName << ".addItem(";

        if (pixmap != 0) {
            refreshOut << pixCall(pixmap);

            if (text)
                refreshOut << ", ";
        }

        refreshOut << trCall(text->elementString()) << ")\n";
    }
}

void WriteInitialization::initializeListWidget(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    QList<DomItem*> items = w->elementItem();

    if (items.isEmpty())
        return;

    refreshOut << option.indent << varName << ".clear()\n";

    // items
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        QString itemName = driver->unique(QLatin1String("__item"));
        refreshOut << "\n";
        refreshOut << option.indent << itemName << " = Qt::ListWidgetItem.new(" << varName << ")\n";

        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);

            if (p->attributeName() == QLatin1String("text"))
                refreshOut << option.indent << itemName << ".setText(" << trCall(p->elementString()) << ")\n";

            if (p->attributeName() == QLatin1String("icon"))
                refreshOut << option.indent << itemName << ".setIcon(" << pixCall(p) << ")\n";
        }
    }
}

void WriteInitialization::initializeTreeWidget(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    // columns
    QList<DomColumn*> columns = w->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *icon = properties.value(QLatin1String("icon"));

        QString txt = trCall(text->elementString());
        refreshOut << option.indent << varName << ".headerItem().setText(" << i << ", " << txt << ")\n";

        if (icon != 0 && icon->elementIconSet()) {
            output << option.indent << varName << ".headerItem().setIcon("
                   << i << ", " << pixCall(icon) << ")\n";
        }
    }

    if (w->elementItem().size()) {
        refreshOut << option.indent << varName << ".clear()\n";

        initializeTreeWidgetItems(className, varName, w->elementItem());
    }
}

void WriteInitialization::initializeTableWidget(DomWidget *w)
{
    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));
    QString className = w->attributeClass();

    // columns
    QList<DomColumn *> columns = w->elementColumn();

    if (columns.size() != 0) {
        refreshOut << option.indent << "if " << varName << ".columnCount() < " << columns.size() << "\n"
            << option.indent << option.indent << varName << ".setColumnCount(" << columns.size() << ")\n"
            << option.indent << "end\n";
    }

    for (int i = 0; i < columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *icon = properties.value(QLatin1String("icon"));
        if (text || icon) {
            QString itemName = driver->unique(QLatin1String("__colItem"));
            refreshOut << "\n";
            refreshOut << option.indent 
                           << itemName << " = Qt::TableWidgetItem.new\n";

            if (text && text->attributeName() == QLatin1String("text"))
                refreshOut << option.indent << itemName << ".setText("
                           << trCall(text->elementString()) << ")\n";

            if (icon && icon->attributeName() == QLatin1String("icon"))
                refreshOut << option.indent << itemName << ".setIcon("
                           << pixCall(icon) << ")\n";
            refreshOut << option.indent << varName << ".setHorizontalHeaderItem("
                           << i << ", " << itemName << ")\n";
        }
    }

    // rows
    QList<DomRow *> rows = w->elementRow();

    if (rows.size() != 0) {
        refreshOut << option.indent << "if " << varName << ".rowCount() < " << rows.size() << "\n"
            << option.indent << option.indent << varName << ".setRowCount(" << rows.size() << ")\n"
            << option.indent << "end\n";
    }

    for (int i = 0; i < rows.size(); ++i) {
        DomRow *row = rows.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(row->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *icon = properties.value(QLatin1String("icon"));
        if (text || icon) {
            QString itemName = driver->unique(QLatin1String("__rowItem"));
            refreshOut << "\n";
            refreshOut << option.indent
                           << itemName << " = Qt::TableWidgetItem.new\n";

            if (text && text->attributeName() == QLatin1String("text"))
                refreshOut << option.indent << itemName << ".setText("
                           << trCall(text->elementString()) << ")\n";

            if (icon && icon->attributeName() == QLatin1String("icon"))
                refreshOut << option.indent << itemName << ".setIcon("
                           << pixCall(icon) << ")\n";
            refreshOut << option.indent << varName << ".setVerticalHeaderItem("
                           << i << ", " << itemName << ")\n";
        }
    }

    // items
    QList<DomItem *> items = w->elementItem();
    for (int i = 0; i < items.size(); ++i) {
        DomItem *item = items.at(i);
        if (item->hasAttributeRow() && item->hasAttributeColumn()) {
            QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
            DomProperty *text = properties.value(QLatin1String("text"));
            DomProperty *icon = properties.value(QLatin1String("icon"));
            if (text || icon) {
                QString itemName = driver->unique(QLatin1String("__item"));
                refreshOut << "\n";
                refreshOut << option.indent
                    << itemName << " = Qt::TableWidgetItem.new\n";

                if (text && text->attributeName() == QLatin1String("text"))
                    refreshOut << option.indent << itemName << ".setText("
                        << trCall(text->elementString()) << ")\n";

                if (icon && icon->attributeName() == QLatin1String("icon"))
                    refreshOut << option.indent << itemName << ".setIcon("
                        << pixCall(icon) << ")\n";
                refreshOut << option.indent << varName << ".setItem("
                    << item->attributeRow() << ", "
                    << item->attributeColumn() << ", "
                    << itemName << ")\n";
            }
        }
    }
}

QString WriteInitialization::trCall(const QString &str, const QString &commentHint) const
{
    if (str.isEmpty())
        return QLatin1String("''");

    QString result;
    QString comment = commentHint.isEmpty() ? QString::fromUtf8("nil") : fixString(commentHint, option.indent);

    if (option.translateFunction.isEmpty()) {
        result = QLatin1String("Qt::Application.translate(\"");
        result += m_generatedClass;
        result += QLatin1String("\"");
        result += QLatin1String(", ");
    } else {
        result = option.translateFunction + QLatin1String("(");
    }

    result += fixString(str, option.indent);
    result += QLatin1String(", ");
    result += comment;

    if (option.translateFunction.isEmpty()) {
        result += QLatin1String(", ");
        result += QLatin1String("Qt::Application::UnicodeUTF8");
    }

    result += QLatin1String(")");
    return result;
}

void WriteInitialization::initializeQ3SqlDataTable(DomWidget *w)
{
    QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value(QLatin1String("database"), 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));

    output << option.indent << "if !" << varName << ".sqlCursor().nil?\n";

    output << option.indent << option.indent << varName << ".setSqlCursor(";

    if (connection == QLatin1String("(default)")) {
        output << "Qt3::SqlCursor.new(" << fixString(table, option.indent) << "), false, true)\n";
    } else {
        output << "Qt3::SqlCursor.new(" << fixString(table, option.indent) << ", true, " << connection << "Connection" << "), false, true)\n";
    }
    output << option.indent << option.indent  << varName << ".refresh(Qt3::DataTable::RefreshAll)\n";
    output << option.indent << "end\n";}

void WriteInitialization::initializeQ3SqlDataBrowser(DomWidget *w)
{
    QHash<QString, DomProperty*> properties = propertyMap(w->elementProperty());

    DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    QString connection;
    QString table;
    QString field;

    DomProperty *db = properties.value(QLatin1String("database"), 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();
        connection = info.size() > 0 ? info.at(0) : QString();
        table = info.size() > 1 ? info.at(1) : QString();
        field = info.size() > 2 ? info.at(2) : QString();
    }

    if (table.isEmpty() || connection.isEmpty()) {
        fprintf(stderr, "invalid database connection\n");
        return;
    }

    QString varName = toRubyIdentifier(driver->findOrInsertWidget(w));

    output << option.indent << "if !" << varName << ".sqlCursor().nil?\n";

    output << option.indent << option.indent << varName << ".setSqlCursor(";

    if (connection == QLatin1String("(default)")) {
        output << "Qt::SqlCursor.new(" << fixString(table, option.indent) << "), true);\n";
    } else {
        output << "Qt::SqlCursor.new(" << fixString(table, option.indent) << ", true, " << connection << "Connection" << "), false, true)\n";
    }
    output << option.indent << option.indent << varName << ".refresh()\n";
    output << option.indent << "end\n";
}

void WriteInitialization::initializeMenu(DomWidget *w, const QString &/*parentWidget*/)
{
    QString menuName = driver->findOrInsertWidget(w);
    QString menuAction = menuName + QLatin1String("Action");

    DomAction *action = driver->actionByName(menuAction);
    if (action && action->hasAttributeMenu()) {
        output << option.indent << menuAction << " = " << menuName << ".menuAction()\n";
    }
}

QString WriteInitialization::trCall(DomString *str) const
{
    return trCall(toString(str), str->attributeComment());
}

bool WriteInitialization::isValidObject(const QString &name) const
{
    return m_registeredWidgets.contains(name)
        || m_registeredActions.contains(name);
}

QString WriteInitialization::findDeclaration(const QString &name)
{
    QString normalized = Driver::normalizedName(name);

    if (DomWidget *widget = driver->widgetByName(normalized))
        return driver->findOrInsertWidget(widget);
    else if (DomAction *action = driver->actionByName(normalized))
        return driver->findOrInsertAction(action);

    return QString();
}

void WriteInitialization::acceptConnection(DomConnection *connection, const QString& mainVar)
{
    QString sender = findDeclaration(connection->elementSender());
    sender = sender.mid(0, 1).toLower() + sender.mid(1);
    if (sender != mainVar) {
		sender = toRubyIdentifier(sender);
    }

    QString receiver = findDeclaration(connection->elementReceiver());
    receiver = receiver.mid(0, 1).toLower() + receiver.mid(1);
    if (receiver != mainVar) {
		receiver = toRubyIdentifier(receiver);
    }

    if (sender.isEmpty() || receiver.isEmpty())
        return;

    output << option.indent << "Qt::Object.connect("
        << sender
        << ", "
        << "SIGNAL('" << connection->elementSignal() << "')"
        << ", "
        << receiver
        << ", "
        << "SLOT('" << connection->elementSlot() << "')"
        << ")\n";
}

DomImage *WriteInitialization::findImage(const QString &name) const
{
    return m_registeredImages.value(name);
}

DomWidget *WriteInitialization::findWidget(const QString &widgetClass)
{
    for (int i = m_widgetChain.count() - 1; i >= 0; --i) {
        DomWidget *widget = m_widgetChain.at(i);

        if (widget && uic->customWidgetsInfo()->extends(widget->attributeClass(), widgetClass))
            return widget;
    }

    return 0;
}

void WriteInitialization::acceptImage(DomImage *image)
{
    if (!image->hasAttributeName())
        return;

    m_registeredImages.insert(image->attributeName(), image);
}

} // namespace Ruby
