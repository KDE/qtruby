#include <ruby.h>

/***************************************************************************
       krubyinit  -  makes use of kdeinit_wrapper possible for ruby programs
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

int main(int argc, char **argv) {
     ruby_init();
     ruby_script("embedded");
     ruby_options(argc, argv);
     ruby_run();
}
