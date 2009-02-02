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
#include <QDir>

#include <KStandardDirs>
#include <KComponentData>
#include <kdebug.h>

// Allows KDE Ruby applications to be launched via a .desktop file.
// Add an entry like this to the .desktop file:
//
//     Exec=krubyapplication dbpedia_references/dbpedia_references.rb
//
// Giving the name of the application's directory and the top level script.
//
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s application_directory/ruby_script [Qt-options] [KDE-options]\n", argv[0]);
        return 1;
    }

    QFileInfo script(argv[1]);
    KComponentData componentData(script.dir().dirName().toLatin1(), QByteArray(), KComponentData::SkipMainComponentRegistration);
    QString path = componentData.dirs()->locate("data", argv[1], componentData);

    if (path.isEmpty()) {
        kWarning() << "Ruby script" << argv[1] << "missing";
        return 1;
    }

    QFileInfo program(path);
     
    char ** rubyargs = (char **) calloc(argc+1, sizeof(char *));
    rubyargs[0] = strdup(argv[0]);
    rubyargs[1] = strdup("-KU");
    rubyargs[2] = strdup(QFile::encodeName(program.filePath()));
    for (int i = 2; i < argc; i++) {
        rubyargs[i+1] = strdup(argv[i]);
    }

#ifdef RUBY_INIT_STACK
    RUBY_INIT_STACK
#endif
    ruby_init();
    ruby_init_loadpath();
    ruby_incpush(QFile::encodeName(program.path()));
#if RUBY_VERSION < 0x10900
    ruby_options(argc+1, rubyargs); 
    ruby_script(QFile::encodeName(program.fileName()));
    ruby_run();
#else
    ruby_script(QFile::encodeName(program.fileName()));
    ruby_run_node(ruby_options(argc+1, rubyargs));
#endif
}
