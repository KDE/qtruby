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

#ifndef UTILS_H
#define UTILS_H

#include "ui4.h"

#include <qstring.h>
#include <qlist.h>
#include <qhash.h>

inline bool toBool(const QString &str)
{ return str.toLower() == QLatin1String("true"); }

inline QString toString(const DomString *str)
{ return str ? str->text() : QString(); }

inline QString fixString(const QString &str)
{
	QByteArray utf8 = str.toUtf8();
	uchar cbyte;
    QString result;

    for (int i = 0; i < utf8.length(); ++i) {
		cbyte = utf8.at(i);
		if (cbyte >= 0x80) {
			result += QLatin1String("\\") + QString::number(cbyte, 8);
		} else {
			switch(cbyte) {
			case '\\':
				result += QLatin1String("\\\\"); break;
			case '\"':
				result += QLatin1String("\\\""); break;
			case '\r':
				break;
			case '\n':
				result += QLatin1String("\\n\"\n\""); break;
			default:
				result += QChar(cbyte);
			}
		}
    }

	return QLatin1String("\"") + result + QLatin1String("\"");
}

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties)
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}

inline QStringList unique(const QStringList &lst)
{
    QHash<QString, bool> h;
    for (int i=0; i<lst.size(); ++i)
        h.insert(lst.at(i), true);
    return h.keys();
}

#endif
