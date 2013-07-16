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

#ifndef QTRUBY_PRETTYPRINT_H
#define QTRUBY_PRETTYPRINT_H

#include <smoke.h>
#include <ruby.h>

#include "qtruby_export.h"

namespace QtRuby {

extern VALUE inspect_qobject(VALUE self);
extern VALUE pretty_print_qobject(VALUE self, VALUE pp);

}

#endif // QTRUBY_PRETTYPRINT_H

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

