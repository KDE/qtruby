/*
 *   Copyright 2009 by Richard Dale <richard.j.dale@gmail.com>

 *   Based on the PerlQt marshalling code by Ashley Winters

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtWebKitWidgets/QWebFrame>

#include <QtWebKit/QWebElement>

#include "marshall.h"
#include "global.h"
#include "rubymetatype.h"

Q_DECLARE_METATYPE(QList<QWebElement>)
Q_DECLARE_METATYPE(QWebFrame*)
Q_DECLARE_METATYPE(QList<QWebFrame*>)

namespace QtRuby {

Marshall::TypeHandler QtWebKitHandlers[] = {
    { "QList<QWebFrame*>", marshall_Container<QList<QWebFrame*> > },
    { "QList<QWebElement>", marshall_Container<QList<QWebElement> > },

    { 0, 0 }
};

void registerQtWebKitTypes()
{
    qRubySmokeRegisterPointerSequenceMetaType<QList<QWebFrame*> >();
    qRubySmokeRegisterSequenceMetaType<QList<QWebElement> >();

    return;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
