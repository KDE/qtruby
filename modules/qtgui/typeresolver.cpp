/*
 *   Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>

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

#include <QtGui/QGraphicsItem>

#include <global.h>

#include "typeresolver.h"

namespace QtRuby {

void
qgraphicsitemTypeResolver(QtRuby::Object::Instance * instance)
{
    Smoke::ModuleIndex classId = instance->classId;
    Smoke * smoke = classId.smoke;
    QGraphicsItem * item = reinterpret_cast<QGraphicsItem*>(instance->cast(QtRuby::Global::QGraphicsItemClassId));
    switch (item->type()) {
    case 1:
        instance->classId = smoke->findClass("QGraphicsItem");
        break;
    case 2:
        instance->classId = smoke->findClass("QGraphicsPathItem");
        break;
    case 3:
        instance->classId = smoke->findClass("QGraphicsRectItem");
    case 4:
        instance->classId = smoke->findClass("QGraphicsEllipseItem");
        break;
    case 5:
        instance->classId = smoke->findClass("QGraphicsPolygonItem");
        break;
    case 6:
        instance->classId = smoke->findClass("QGraphicsLineItem");
        break;
    case 7:
        instance->classId = smoke->findClass("QGraphicsItem");
        break;
    case 8:
        instance->classId = smoke->findClass("QGraphicsTextItem");
        break;
    case 9:
        instance->classId = smoke->findClass("QGraphicsSimpleTextItem");
        break;
    case 10:
        instance->classId = smoke->findClass("QGraphicsItemGroup");
        break;
    }

    instance->value = instance->cast(instance->classId);
    return;
}

}
