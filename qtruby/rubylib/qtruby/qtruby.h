/***************************************************************************
                          qtruby.h  -  description
                             -------------------
    begin                : Fri Jul 4 2003
    copyright            : (C) 2003 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QTRUBY_H
#define QTRUBY_H

#include "marshall.h"

struct smokeruby_object {
    bool allocated;
    Smoke *smoke;
    int classId;
    void *ptr;
};

struct TypeHandler {
    const char *name;
    Marshall::HandlerFn fn;
};

extern int do_debug;   // evil
extern VALUE rv_qapp;
extern int object_count;

// keep this enum in sync with lib/Qt/debug.pm

enum QtDebugChannel {
    qtdb_none = 0x00,
    qtdb_ambiguous = 0x01,
    qtdb_autoload = 0x02,
    qtdb_calls = 0x04,
    qtdb_gc = 0x08,
    qtdb_virtual = 0x10,
    qtdb_verbose = 0x20
};

void unmapPointer(smokeruby_object *, Smoke::Index, void*);
smokeruby_object *value_obj_info(VALUE value);
VALUE getPointerObject(void *ptr);

#endif
