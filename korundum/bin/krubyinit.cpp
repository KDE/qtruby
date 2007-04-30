#include <ruby.h>

/***************************************************************************
       krubyinit  -  makes use of kdeinit4_wrapper possible for ruby programs
                             -------------------
    begin                : Wed Jan 7 2004
    copyright            : (C) 2004 by Alexander Kellett
    email                : lypanov@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// this name can be used to allow apps 
// to detect what they were started with
static const char* script_name = "krubyinit_app";

int main(int argc, char **argv) {
     ruby_init();
     ruby_script((char*)script_name);
     ruby_options(argc, argv); 
     ruby_run();
}
