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

#include <qtextstream.h>

#include "writeiconinitialization.h"
#include "writeicondata.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"

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
                "imageData.length, " << fixString(fmt) << ")\n";
        output << option.indent << option.indent << option.indent << 
                "return Qt::Pixmap.fromImage(img)\n";
    }
}

