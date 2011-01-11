/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "rbwriteiconinitialization.h"
#include "rbwriteicondata.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"

#include <QtCore/QTextStream>
#include <QtCore/QString>

#if defined(QT_BEGIN_NAMESPACE)
  QT_BEGIN_NAMESPACE
#endif

namespace Ruby {

WriteIconInitialization::WriteIconInitialization(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
    this->uic = uic;
}

void WriteIconInitialization::acceptUI(DomUI *node)
{
    if (node->elementImages() == 0)
        return;

    QString className = node->elementClass() + option.postfix;

    output << option.indent << "def self.icon(id)\n";

    WriteIconData(uic).acceptUI(node);

    output << option.indent << option.indent << "case id\n";

    TreeWalker::acceptUI(node);

    output << option.indent << option.indent << "else\n";
    output << option.indent << option.indent << option.indent << "return Qt::Pixmap.new\n";

    output << option.indent << option.indent << "end\n"
           << option.indent << "end\n\n";
}

QString WriteIconInitialization::iconFromDataFunction()
{
    return QLatin1String("qt_get_icon");
}

void WriteIconInitialization::acceptImages(DomImages *images)
{
    TreeWalker::acceptImages(images);
}

void WriteIconInitialization::acceptImage(DomImage *image)
{
    QString img = image->attributeName() + QLatin1String("_data");
    QString data = image->elementData()->text();
    QString fmt = image->elementData()->attributeFormat();

    QString imageId = image->attributeName() + QLatin1String("_ID");
    QString imageData = image->attributeName() + QLatin1String("_data");
    QString ind = option.indent + option.indent;

    output << ind << "when " << imageId << "\n";

    if (fmt == QLatin1String("XPM.GZ")) {
        output << option.indent << option.indent << option.indent << "return " << "Qt::Pixmap.new(" << imageData << ")\n";
    } else {
        output << option.indent << option.indent << option.indent << 
                " img = Qt::Image.new\n";
        output << option.indent << option.indent << option.indent << "img.loadFromData(" << imageData << ", " << 
                "imageData.length, " << fixString(fmt, ind) << ")\n";
        output << option.indent << option.indent << option.indent << 
                "return Qt::Pixmap.fromImage(img)\n";
    }
}

} // namespace Ruby

#if defined(QT_END_NAMESPACE)
  QT_END_NAMESPACE
#endif
