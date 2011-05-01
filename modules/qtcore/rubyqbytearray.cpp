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

#include <QtCore/QByteArray>

#include <object.h>
#include <global.h>

#include "rubyqbytearray.h"

namespace QtRuby {

// There is a QByteArray operator method in the Smoke lib that takes a QString
// arg and returns a QString. This is normally the desired behaviour, so
// special case a '+' method here.
VALUE
qbytearray_append(VALUE self, VALUE str)
{
    Object::Instance * instance = Object::Instance::get(self);
    QByteArray * bytes = reinterpret_cast<QByteArray *>(instance->value);
    (*bytes) += (const char *) StringValuePtr(str);
    return self;
}

VALUE
qbytearray_data(VALUE self)
{
  Object::Instance * instance = Object::Instance::get(self);
  QByteArray * bytes = reinterpret_cast<QByteArray *>(instance->value);
  return rb_str_new(bytes->data(), bytes->size());
}

}