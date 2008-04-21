/***************************************************************************
 *   Copyright (C) 2008 by Richard Dale <richard.j.dale@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <ruby.h>

#include <QString>
#include <QFileInfo>

#include <KStandardDirs>
#include <KComponentData>
#include <kdebug.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s application_name ruby_script\n", argv[0]);
        return 1;
    }

    KComponentData componentData(argv[1]);
    QString path = componentData.dirs()->locate("data", argv[2]);

    if (path.isEmpty()) {
        kWarning() << "Ruby script" << argv[1] << "missing";
        return 1;
    }

    QFileInfo program(path);
     
    char ** rubyargs = (char **) calloc(argc-1, sizeof(char *));
    rubyargs[0] = strdup(program.fileName().latin1());
    rubyargs[1] = strdup(program.fileName().latin1());
    for (int i = 3; i < argc; i++) {
        rubyargs[i-1] = argv[i];
    }

    RUBY_INIT_STACK
    ruby_init();
    ruby_options(argc-1, rubyargs); 
    ruby_init_loadpath();
    ruby_incpush(QFile::encodeName(program.path()));
    rb_gv_set("$KCODE", rb_str_new2("u"));
    ruby_run();
}
