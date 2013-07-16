/*
 * Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QTRUBY_RUBYQMETAOBJECT_H
#define QTRUBY_RUBYQMETAOBJECT_H

#include <smoke.h>
#include <ruby.h>

#include "qtruby_export.h"

namespace QtRuby {
    extern VALUE ruby_slots(int argc, VALUE * argv, VALUE self);
    extern VALUE ruby_private_slots(int argc, VALUE * argv, VALUE self);
    extern VALUE ruby_signals(int argc, VALUE * argv, VALUE self);
    extern VALUE ruby_classinfo(VALUE self, VALUE key, VALUE value);
    extern VALUE qt_metacall(int /*argc*/, VALUE * argv, VALUE self);
    extern VALUE qt_signal(int argc, VALUE * argv, VALUE self);
    extern VALUE metaObject(VALUE self);
}

#endif // QTRUBY_RUBYQMETAOBJECT_H
