/*
 *   Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>

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

#include <QtGui/QImage>

#include <object.h>
#include <global.h>

#include "rubyqimage.h"

namespace QtRuby {

VALUE
qimage_bits(VALUE self)
{
  Object::Instance * instance = Object::Instance::get(self);
  QImage * image = reinterpret_cast<QImage*>(instance->value);
  const uchar * bytes = image->bits();
  return rb_str_new((const char *) bytes, image->numBytes());
}

VALUE
qimage_scan_line(VALUE self, VALUE ix)
{
  Object::Instance * instance = Object::Instance::get(self);
  QImage * image = reinterpret_cast<QImage*>(instance->value);
  const uchar * bytes = image->scanLine(NUM2INT(ix));
  return rb_str_new((const char *) bytes, image->bytesPerLine());
}

}