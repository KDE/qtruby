/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
** Copyright (c) 2001 Phil Thompson <phil@river-bank.demon.co.uk>
** Copyright (c) 2002 Germain Garand <germain@ebooksfrance.com>
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*
** 06/2002 : Initial release of puic, the PerlQt User Interface Compiler,
**           a work derivated from uic (the Qt User Interface Compiler)
**           and pyuic (the PyQt User Interface Compiler).
**
**           G.Garand
**
** 08/2003 : Initial release of rbuic, the QtRuby User Interface Compiler,
**           a work derived from the PerlQt puic.
**
**           Richard Dale
**
**********************************************************************/
#include "uic.h"
#include "parser.h"
#include "widgetdatabase.h"
#include "domtool.h"
#include <qapplication.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qdatetime.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#define RBUIC_VERSION "0.9"

void getDBConnections( Uic& uic, QString& s);

int main( int argc, char * argv[] )
{
    RubyIndent indent;
    bool execCode = FALSE;
    bool subcl = FALSE;
    bool imagecollection = FALSE;
    QStringList images;
    const char *error = 0;
    const char* fileName = 0;
    const char* className = 0;
    const char* outputFile = 0;
    const char* projectName = 0;
    const char* trmacro = 0;
    bool nofwd = FALSE;
    bool useKDE = FALSE;
    bool fix = FALSE;
    QApplication app(argc, argv, FALSE);
    QString uicClass;


    for ( int n = 1; n < argc && error == 0; n++ ) {
	QCString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    QCString opt = &arg[1];
	    if ( opt[0] == 'o' ) {		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output-file name";
			break;
		    }
		    outputFile = argv[++n];
		} else
		    outputFile = &opt[1];
	    } else if ( opt[0] == 'e' || opt == "embed" ) {
		imagecollection = TRUE;
		if ( opt == "embed" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing name of project.";
			break;
		    }
		    projectName = argv[++n];
		} else
		    projectName = &opt[1];
	    } else if ( opt == "nofwd" ) {
		nofwd = TRUE;
	    } else if ( opt == "kde" ) {
		useKDE = TRUE;
	    } else if ( opt == "subimpl" ) {
		subcl = TRUE;
		if ( !(n < argc-1) ) {
		    error = "Missing class name.";
		    break;
		}
		className = argv[++n];
	    } else if ( opt == "tr" ) {
		if ( opt == "tr" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing tr macro.";
			break;
		    }
		    trmacro = argv[++n];
		} else {
		    trmacro = &opt[1];
		}
	    } else if ( opt == "version" ) {
		fprintf( stderr,
			 "QtRuby User Interface Compiler v%s for Qt version %s\n", RBUIC_VERSION,
			 QT_VERSION_STR );
		exit( 1 );
	    } else if ( opt == "help" ) {
		break;
	    } else if ( opt == "fix" ) {
		fix = TRUE;
	    } else if ( opt[0] == 'p' ) {
		uint tabstop;
		bool ok;

		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing indent";
			break;
		    }
		    tabstop = QCString(argv[++n]).toUInt(&ok);
		} else
		    tabstop = opt.mid(1).toUInt(&ok);

		if (ok)
		    indent.setTabStop(tabstop);
		else
		    error = "Invalid indent";
	    } else if ( opt == "x" ) {
		execCode = TRUE;
	    } else {
		error = "Unrecognized option";
	    }
	} else {
	    if ( imagecollection )
		images << argv[n];
	    else if ( fileName )		// can handle only one file
		error	 = "Too many input files specified";
	    else
		fileName = argv[n];
	}
    }

    if ( argc < 2 || error || (!fileName && !imagecollection ) ) {
	fprintf( stderr, "QtRuby user interface compiler.\n" );
	if ( error )
	    fprintf( stderr, "rbuic: %s\n", error );

	fprintf( stderr, "Usage: %s  [options] [mode] <uifile>\n"
		 "\nGenerate implementation:\n"
		 "   %s  [options] <uifile>\n"
		 "Generate image collection:\n"
		 "   %s  [options] -embed <project> <image1> <image2> <image3> ...\n"
		 "\t<project>\tproject name\n"
		 "\t<image[0..n]>\timage files\n"
		 "Generate subclass implementation:\n"
		 "   %s  [options] -subimpl <classname> <uifile>\n"
		 "\t<classname>\tname of the subclass to generate\n"
		 "Options:\n"
		 "\t-o file\t\tWrite output to file rather than stdout\n"
		 "\t-p indent\tSet the indent in spaces (0 to use a tab)\n"
		 "\t-nofwd\t\tOmit imports of custom widgets\n"
		 "\t-kde\t\tUse kde widgets, require 'Korundum' extension\n"
		 "\t-tr func\tUse func(...) rather than trUtf8(...) for i18n\n"
		 "\t-x\t\tGenerate extra code to test the class\n"
		 "\t-version\tDisplay version of rbuic\n"
		 "\t-help\t\tDisplay this information\n"
		 , argv[0], argv[0], argv[0], argv[0]);
	exit( 1 );
    }

    Uic::setIndent(indent);

    QFile fileOut;
    if ( outputFile ) {
	fileOut.setName( outputFile );
	if (!fileOut.open( IO_WriteOnly ) )
	    qFatal( "rbuic: Could not open output file '%s'", outputFile );
    } else {
	fileOut.open( IO_WriteOnly, stdout );
    }
    QTextStream out( &fileOut );

    if ( imagecollection ) {
	out.setEncoding( QTextStream::Latin1 );
	Uic::embed( out, projectName, images );
	return 0;
    }


    out.setEncoding( QTextStream::UnicodeUTF8 );
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) )
	qFatal( "rbuic: Could not open file '%s' ", fileName );

    QDomDocument doc;
    QString errMsg;
    int errLine;
    if ( !doc.setContent( &file, &errMsg, &errLine ) )
	qFatal( QString("rbuic: Failed to parse %s: ") + errMsg + QString (" in line %d\n"), fileName, errLine );

    DomTool::fixDocument( doc );

    if ( fix ) {
	out << doc.toString();
	return 0;
    }

    if ( !subcl ) {
	out << "# Form implementation generated from reading ui file '" << fileName << "'" << endl;
	out << "#" << endl;
	out << "# Created: " << QDateTime::currentDateTime().toString() << endl;
	out << "#      by: The QtRuby User Interface Compiler (rbuic)" << endl;
	out << "#" << endl;
	out << "# WARNING! All changes made in this file will be lost!" << endl;
	out << endl;
    }
    out << endl;

    Uic uic( fileName, out, doc, subcl, trmacro ? trmacro : "trUtf8", className, nofwd, uicClass, useKDE );

    if (execCode) {
    out << endl;
	out << indent << "if $0 == __FILE__" << endl;
	++indent;
	if (uic.hasKDEwidget) {
		out << indent << "about = KDE::AboutData.new(\"" << uicClass << "\", \"" << uicClass << "\", \"0.1\")" << endl;
		out << indent << "KDE::CmdLineArgs.init(ARGV, about)" << endl;
		out << indent << "a = KDE::Application.new()" << endl;
	} else {
		out << indent << "a = Qt::Application.new(ARGV)" << endl;
	}
        QString s;
        getDBConnections( uic, s);
        out << s;
	out << indent << "w = " << (subcl? QString::fromLatin1(className) : uicClass) << ".new" << endl;
	out << indent << "a.setMainWidget(w)" << endl;
	out << indent << "w.show" << endl;
	out << indent << "a.exec" << endl;
	--indent;
	out << indent << "end" << endl;
    }

    return 0;
}

void getDBConnections( Uic& uic, QString& s)
{
    int num = 0;
    for ( QStringList::Iterator it = uic.dbConnections.begin(); it != uic.dbConnections.end(); ++it ) {
        if ( !(*it).isEmpty()) {
            QString inc = (num ? QString::number(num+1) : QString::null);
            s += "\n# Connection to database " + (*it) + "\n\n";
            s += "DRIVER" + inc + " =\t\t'QMYSQL3'" + (inc?"":" # appropriate driver") + "\n";
            s += "DATABASE" + inc + " =\t\t'foo'" + (inc?"":" # name of your database") + "\n";
            s += "USER" + inc + "=\t\t'john'" + (inc?"":" # username") + "\n";
            s += "PASSWORD" + inc + "=\t\t'ZxjGG34s'" + (inc?"":" # password for USER") + "\n";
            s += "HOST" + inc + "=\t\t'localhost'" + (inc?"":" # host on which the database is running") + "\n";
            s += "\n";
            s += "db" + inc + " = Qt::SqlDatabase.addDatabase( DRIVER" + inc;
            if (inc)
                s+= ", '" + (*it) + "'";
            s += " )\n";
            s += "   db" + inc + ".setDatabaseName( DATABASE" + inc + " )\n";
            s += "   db" + inc + ".setUserName( USER" + inc + " )\n";
            s += "   db" + inc + ".setPassword( PASSWORD" + inc + " )\n";
            s += "   db" + inc + ".setHostName( HOST" + inc + " )\n";
            s += "\n";
            s += "if!db" + inc + ".open() \n";
             s += "        Qt::MessageBox.information( undef, 'Unable to open database',\n";
            s += "                                     db" + inc + ".lastError().databaseText() . \"\\n\")\n";
            s += "        exit 1\n";
            s += "end\n";
            s += "\n";
            num++;
        }
    }
}

