/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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
**********************************************************************/

#include "uic.h"
#include "parser.h"
#include "widgetdatabase.h"
#include "domtool.h"

#include <qstringlist.h>
#include <qregexp.h>
#include <qfile.h>
#include <qfileinfo.h>

#define NO_STATIC_COLORS
#include <globaldefs.h>

#include <zlib.h>

static QByteArray unzipXPM( QString data, ulong& length )
{
    const int lengthOffset = 4;
    int baSize = data.length() / 2 + lengthOffset;
    uchar *ba = new uchar[ baSize ];
    for ( int i = lengthOffset; i < baSize; ++i ) {
        char h = data[ 2 * (i-lengthOffset) ].latin1();
        char l = data[ 2 * (i-lengthOffset) + 1 ].latin1();
	uchar r = 0;
	if ( h <= '9' )
	    r += h - '0';
	else
	    r += h - 'a' + 10;
	r = r << 4;
	if ( l <= '9' )
	    r += l - '0';
	else
	    r += l - 'a' + 10;
	ba[ i ] = r;
    }
    // qUncompress() expects the first 4 bytes to be the expected length of the
    // uncompressed data 
    ba[0] = ( length & 0xff000000 ) >> 24;
    ba[1] = ( length & 0x00ff0000 ) >> 16;
    ba[2] = ( length & 0x0000ff00 ) >> 8;
    ba[3] = ( length & 0x000000ff );
    QByteArray baunzip = qUncompress( ba, baSize );
    delete[] ba;
    return baunzip;
}

static QString imageDataName(QString name) {
	QString result = name + "_data";
	result.replace("@", "@@");
	return result;
}

/*!
  Creates an implementation ( cpp-file ) for the form given in \a e

  \sa createFormDecl(), createObjectImpl()
 */
void Uic::createFormImpl( const QDomElement &e )
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;
    QString objName = getObjectName( e );
	
	if (hasKDEwidget) {
    	out << indent << "require 'Korundum'" << endl << endl;
	} else {
    	out << indent << "require 'Qt'" << endl << endl;
	}

    // generate local and local includes required
    QStringList globalIncludes, localIncludes;
    QStringList::Iterator it;
    QStringList sqlClasses;

    QMap<QString, CustomInclude> customWidgetIncludes;
    QMap<QString, QString> functionImpls;

    // find additional slots
    QStringList extraSlots;
    QStringList extraSlotTypes;
    nl = e.parentNode().toElement().elementsByTagName( "slot" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "slots"
	     && n.parentNode().toElement().tagName() != "connections" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString slotName = n.firstChild().toText().data().stripWhiteSpace();
	if ( slotName.endsWith( ";" ) )
	    slotName = slotName.left( slotName.length() - 1 );

	extraSlots += Parser::cleanArgs(slotName);
	extraSlotTypes += n.attribute( "returnType", "void" );
    }

    // find signals
    QStringList extraSignals;
    nl = e.parentNode().toElement().elementsByTagName( "signal" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "signals"
	     && n.parentNode().toElement().tagName() != "connections" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString sigName = n.firstChild().toText().data().stripWhiteSpace();
	if ( sigName.endsWith( ";" ) )
	    sigName = sigName.left( sigName.length() - 1 );
	extraSignals += sigName;
    }

    //find additional functions
    QStringList extraFunctions;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
        if ( n.tagName() == "functions" ) { // compatibility
            for ( QDomElement n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
                if ( n2.tagName() == "function" ) {
                    QString fname;
                    if( !n2.attribute("name").isNull() )
                    {
                        fname = n2.attribute( "name" );
                        fname = Parser::cleanArgs( fname );
                        functionImpls.insert( fname, n2.firstChild().toText().data() );
                    }
                    else
                    {
                        fname = n2.text();
                        fname = Parser::cleanArgs( fname );
                    }
                    extraFunctions += fname;
                }
            }
        } else if ( n.tagName() == "customwidgets" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "customwidget" ) {
		    QDomElement n3 = n2.firstChild().toElement();
		    QString cl, header;
		    WidgetDatabaseRecord *r = new WidgetDatabaseRecord;
		    while ( !n3.isNull() ) {
			if ( n3.tagName() == "class" ) {
			    cl = n3.firstChild().toText().data();
			    r->name = cl;
			} else if ( n3.tagName() == "header" ) {
			    CustomInclude ci;
			    ci.header = n3.firstChild().toText().data();
			    ci.location = n3.attribute( "location", "global" );
			    r->includeFile = ci.header;
			    header = ci.header;
			    customWidgetIncludes.insert( cl, ci );
			}
			WidgetDatabase::append( r );
			n3 = n3.nextSibling().toElement();
		    }

		    if (cl.isEmpty())
			cl = "UnnamedCustomClass";

		    int ext = header.findRev('.');

		    if (ext >= 0)
			header.truncate(ext);

		    if (header.isEmpty())
			header = cl.lower();

//		    if (!nofwd)
//		    	out << "use " << cl << ";" << endl; // FIXME: what about header ?
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
    }

    out << "class " << nameOfClass << " < " << objClass << endl << endl;

    // QtRuby sig/slot declaration
	
    ++indent;
    
	if ( !extraSlots.isEmpty() ) {
		out << indent << "slots 'languageChange()'";
    	for ( it = extraSlots.begin(); it != extraSlots.end(); ++it ) {
			if (it == extraSlots.begin()) {
				out << "," << endl;
			}
	    	rubySlot( it );
	    	out << ( ((*it) == extraSlots.last()) ? "":",") << endl;
		}
		out << endl;
     }

    // create signals
    if ( !extraSignals.isEmpty() ) {
		out << indent << "signals ";
		--indent;
		for ( it = extraSignals.begin(); it != extraSignals.end(); ++it ) {
	    	rubySlot( it );
			if (it == extraSignals.begin()) {
				++indent;
			}
	    	out << ( ((*it) == extraSignals.last()) ? "":",") << endl;
		}
		out << endl;
    }


    // children
    if( !objectNames.isEmpty() )
    	qWarning("WARNING : objectNames should be empty at form.cpp line %d\n", __LINE__);
    nl = e.parentNode().toElement().elementsByTagName( "widget" );
    for ( i = 1; i < (int) nl.length(); i++ )
    { // start at 1, 0 is the toplevel widget
	n = nl.item(i).toElement();
	createAttrDecl( n );
    }
    objectNames.clear();
	
    ++indent;

    // additional attributes (from Designer)
    QStringList publicVars, protectedVars, privateVars;
    nl = e.parentNode().toElement().elementsByTagName( "variable" );
    for ( i = 0; i < (int)nl.length(); i++ ) {
        n = nl.item( i ).toElement();
        // Because of compatibility the next lines have to be commented out.
        // Someday it should be uncommented.
        //if ( n.parentNode().toElement().tagName() != "variables" )
        //    continue;
        QString access = n.attribute( "access", "protected" );
        QString var = n.firstChild().toText().data().stripWhiteSpace();
        if ( var.endsWith( ";" ) )
            var.truncate(var.length() - 1);
        if ( access == "public" )
            publicVars += var;
        else if ( access == "private" )
            privateVars += var;
        else
            protectedVars += var;
    }

    // Databases Connection holders

    registerDatabases( e );
    dbConnections = unique( dbConnections );
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty() && (*it) != "(default)") {
	    out << indent << (*it) << "Connection" << endl;
	}
    }

    --indent;

    // additional includes (local or global ) and forward declaractions
    nl = e.parentNode().toElement().elementsByTagName( "include" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "location" ) != "local" ) {
	    if ( s.right( 5 ) == ".ui.h" && !QFile::exists( s ) )
		continue;
	    if ( n2.attribute( "impldecl", "in implementation" ) != "in implementation" )
		continue;
	    globalIncludes += s;
	}
    }

    // do the local includes afterwards, since global includes have priority on clashes
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "location" ) == "local" &&!globalIncludes.contains( s ) ) {
	    if ( s.right( 5 ) == ".ui.h" && !QFile::exists( s ) )
		continue;
	    if ( n2.attribute( "impldecl", "in implementation" ) != "in implementation" )
		continue;
	    localIncludes += s;
	}
    }

    // additional custom widget headers
    nl = e.parentNode().toElement().elementsByTagName( "header" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "location" ) != "local" )
	    globalIncludes += s;
	else
	    localIncludes += s;
    }


    // grab slots/funcs defined in ui.h files
    for(QStringList::Iterator it = localIncludes.begin(); it != localIncludes.end(); ++it)
    {
        if((*it).right( 5 ) == ".ui.h")
        {
            QFile f((*it));
            if( f.open( IO_ReadOnly ) )
            {
                QRegExp re("^def\\s+([a-zA-Z0-9_]+)\\s.*$");
                QRegExp re2("^end\\s*$");
                QTextStream t( &f );
                QString s, s2, s3;
                while ( !t.eof() )
                {
                    s = t.readLine();
                    int pos = re.search(s);
                    if(pos == -1)
                        continue;
                    s2 = re.cap(1);
					s2 += "()";
//                    s2 = Parser::cleanArgs(s2);
                    s3 = "{";
                    while( !t.eof() )
                    {
                        s = t.readLine();
                         if(re2.search(s) != -1)
                            break;
                       s3 += s + "\n";
                    }
					s3 += "}";
                    functionImpls.insert( s2, s3 );
                    if( t.eof() ) break;
                }
                f.close();
             }
        }
    }

    // includes for child widgets
    for ( it = tags.begin(); it != tags.end(); ++it ) {
	nl = e.parentNode().toElement().elementsByTagName( *it );
	for ( i = 1; i < (int) nl.length(); i++ ) { // start at 1, 0 is the toplevel widget
	    QString name = getClassName( nl.item(i).toElement() );
	    if ( name == "Spacer" ) {
		globalIncludes += "qlayout.h";
		globalIncludes += "qapplication.h";
		continue;
	    }
	    if ( name.mid( 4 ) == "ListView" )
		globalIncludes += "qheader.h";
	    if ( name != objClass ) {
		int wid = WidgetDatabase::idFromClassName( name.replace( QRegExp("^Qt::"), "Q" ) );
		QMap<QString, CustomInclude>::Iterator it = customWidgetIncludes.find( name );
		if ( it == customWidgetIncludes.end() )
		    globalIncludes += WidgetDatabase::includeFile( wid );
	    }
	}
    }

    dbConnections = unique( dbConnections );
    if ( dbConnections.count() )
	sqlClasses += "Qt::SqlDatabase";
    if ( dbCursors.count() )
	sqlClasses += "Qt::SqlCursor";
    bool dbForm = FALSE;
    if ( dbForms[ "(default)" ].count() )
	dbForm = TRUE;
    bool subDbForms = FALSE;
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty()  && (*it) != "(default)" ) {
	    if ( dbForms[ (*it) ].count() ) {
		subDbForms = TRUE;
		break;
	    }
	}
    }
    if ( dbForm || subDbForms ) {
	sqlClasses += "Qt::SqlForm";
	sqlClasses += "Qt::SqlRecord";
    }

    if (globalIncludes.findIndex("qdatatable.h") >= 0)
        sqlClasses += "Qt::DataTable";

    if (globalIncludes.findIndex("qtableview.h") >= 0)
        sqlClasses += "Qt::TableView";

    if (globalIncludes.findIndex("qdatabrowser.h") >= 0)
        sqlClasses += "Qt::DataBrowser";

    out << endl;

    // find out what images are required
    QStringList requiredImages;
    static const char *imgTags[] = { "pixmap", "iconset", 0 };
    for ( i = 0; imgTags[i] != 0; i++ ) {
       nl = e.parentNode().toElement().elementsByTagName( imgTags[i] );
       for ( int j = 0; j < (int) nl.length(); j++ ) {
           QString img = "@";
           requiredImages += (img + nl.item(j).firstChild().toText().data());
		}
    }

    // register the object and unify its name
	QString loadFunction(objName);
    objName = registerObject( objName );

    QStringList images;
    QStringList xpmImages;
    if ( pixmapLoaderFunction.isEmpty() && !externPixmaps )
    {
	// create images
	for ( n = e; !n.isNull(); n = n.nextSibling().toElement() )
        {
	    if ( n.tagName()  == "images" )
            {
		nl = n.elementsByTagName( "image" );
		for ( i = 0; i < (int) nl.length(); i++ )
                {
		    QString img = registerObject(  nl.item(i).toElement().attribute( "name" ) );
		    if ( !requiredImages.contains( img ) )
			continue;
		    QDomElement tmp = nl.item(i).firstChild().toElement();
		    if ( tmp.tagName() != "data" )
			continue;
		    QString format = tmp.attribute("format", "PNG" );
		    QString data = tmp.firstChild().toText().data();
		    if ( format == "XPM.GZ" )
                    {
			xpmImages += img;
			ulong length = tmp.attribute("length").toULong();
			QByteArray baunzip = unzipXPM( data, length );
			// shouldn't we test the initial `length' against the
			// resulting `length' to catch corrupt UIC files?
			int a = 0;
            out << indent << imageDataName(img) << " =\n[";
 
			while ( baunzip[a] != '\"' )
			    a++;
			for ( ; a < (int) length; a++ )
			{
			    char ch;

			    if ((ch = baunzip[a]) == '}')
			    {
				out << "]\n" << endl;

				break;
			    }

			    out << ch;
			}
		    }
                    else
                    {
			images += img;
                        out << indent << imageDataName(img) << " = [ " << endl;
			++indent;
 			int a ;
			for ( a = 0; a < (int) (data.length()/2)-1; a++ ) {
			    out << "0x" << QString(data[2*a]) << QString(data[2*a+1]) << ",";
			    if ( a % 12 == 11 )
				out << endl << "    ";
			    else
				out << " ";
			}
			out << "0x" << QString(data[2*a]) << QString(data[2*a+1]) << " ].pack \"C*\"" << endl;
			--indent;
                        out << endl;
		    }
		}
	    }
	}
	out << endl;
    }
    else if ( externPixmaps )
    {
	/*
	out << indent << "def uic_load_pixmap_" << loadFunction << "( data )" << endl;
	++indent;
	out << indent << "pix = Qt::Pixmap.new()" << endl;
	out << indent << "m = Qt::MimeSourceFactory.defaultFactory().data(data)" << endl;
	out << endl;
	out << indent << "if ! m.nil?" << endl;
	++indent;
	out << indent << "Qt::ImageDrag.decode(m, pix)" << endl;
	--indent;
	out << indent << "end" << endl;
	out << endl;
	out << indent << "return pix" << endl;
	--indent;
	out << indent << "end" << endl;
	out << endl;
	out << endl;
	pixmapLoaderFunction = "uic_load_pixmap_" + loadFunction;
	*/
	pixmapLoaderFunction = "Qt::Pixmap.fromMimeSource";
    }


    // constructor(s)
    if ( objClass == "Qt::Dialog" || objClass == "Qt::Wizard" ) {
    out << indent << "def initialize(*k)" << endl;
    ++indent;
	out << indent << "super(*k)" << endl;
    } else if ( objClass == "Qt::Widget")  {
    out << indent << "def initialize(*k)" << endl;
    ++indent;
	out << indent << "super(*k)" << endl;
    } else if ( objClass == "Qt::MainWindow" ) {
    out << indent << "def initialize(*k)" << endl;
    ++indent;
	out << indent << "super(*k)" << endl;
	isMainWindow = TRUE;
    } else {
    out << indent << "def initialize(*k)" << endl;
    ++indent;
	out << indent << "super(*k)" << endl;
    }

    out << endl;

    // create pixmaps for all images
    if ( !images.isEmpty() ) {
	QStringList::Iterator it;
	for ( it = images.begin(); it != images.end(); ++it ) {
	    out << indent << (*it) << " = Qt::Pixmap.new()" << endl;
	    out << indent << (*it) << ".loadFromData(" << imageDataName(*it) << ", " << imageDataName(*it) << ".length, \"PNG\")" << endl;
	}
        out << endl;
    }
    // create pixmaps for all images
    if ( !xpmImages.isEmpty() ) {
	for ( it = xpmImages.begin(); it != xpmImages.end(); ++it ) {
	    out << indent << (*it) << " = Qt::Pixmap.new(" << imageDataName(*it) << ")" << endl;
	}
	out << endl;
    }

    if ( isMainWindow )
	out << indent << "statusBar()" << endl;

    // set the properties
   QSize geometry( 0, 0 );
    
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    bool stdset = stdsetdef;
	    if ( n.hasAttribute( "stdset" ) )
		stdset = toBool( n.attribute( "stdset" ) );
	    QString prop = n.attribute("name");
	    QDomElement n2 = n.firstChild().toElement();
	    QString value = setObjectProperty( objClass, QString::null, prop, n2, stdset );
	    if ( value.isEmpty() )
		continue;
		
	    if ( prop == "geometry" && n2.tagName() == "rect") {
		QDomElement n3 = n2.firstChild().toElement();
		while ( !n3.isNull() ) {
		    if ( n3.tagName() == "width" )
			geometry.setWidth( n3.firstChild().toText().data().toInt() );
		    else if ( n3.tagName() == "height" )
			geometry.setHeight( n3.firstChild().toText().data().toInt() );
		    n3 = n3.nextSibling().toElement();
		}
	    } else {
		QString call;
		if ( stdset ) {
		    call = mkStdSet( prop ) + "(" + value + ")";
		} else {
		    call = "setProperty(\"" + prop + "\", Qt::Variant.new(" + value + "))";
	    }

		if ( n2.tagName() == "string" ) {
		    trout << indent << call << endl;
		} else if ( prop == "name" ) {
		    out << indent << "if name.nil?" << endl;
		    out << indent << "\t" << call << endl;
		    out << indent << "end" << endl;
		} else {
		    out << indent << call << endl;
		}
	    }
	}
    }

    out << endl;

    // create all children, some forms have special requirements

    if ( objClass == "Qt::Wizard" )
    {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() )
        {
	    if ( tags.contains( n.tagName()  ) )
            {
		QString page = createObjectImpl( n, objClass, "self" );
		QString comment;
		QString label = DomTool::readAttribute( n, "title", "", comment ).toString();
		out << indent << "addPage(" << page << ", "<< trcall( label ) << ")" << endl;
		trout << indent << "setTitle( " << page << ", " << trcall( label, comment ) << " )" << endl;
		QVariant def( FALSE, 0 );
		if ( DomTool::hasAttribute( n, "backEnabled" ) )
		    out << indent << "setBackEnabled(" << page << "," << mkBool( DomTool::readAttribute( n, "backEnabled", def).toBool() ) << ");" << endl;
		if ( DomTool::hasAttribute( n, "nextEnabled" ) )
		    out << indent << "setNextEnabled(" << page << "," << mkBool( DomTool::readAttribute( n, "nextEnabled", def).toBool() ) << ");" << endl;
		if ( DomTool::hasAttribute( n, "finishEnabled" ) )
		    out << indent << "setFinishEnabled(" << page << "," << mkBool( DomTool::readAttribute( n, "finishEnabled", def).toBool() ) << ");" << endl;
		if ( DomTool::hasAttribute( n, "helpEnabled" ) )
		    out << indent << "setHelpEnabled(" << page << "," << mkBool( DomTool::readAttribute( n, "helpEnabled", def).toBool() ) << ");" << endl;
		if ( DomTool::hasAttribute( n, "finish" ) )
		    out << indent << "setFinish( " << page << "," << mkBool( DomTool::readAttribute( n, "finish", def).toBool() ) << ");" << endl;
	    }
	}
    }
    else
    { // standard widgets
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() )
        {
	    if ( tags.contains( n.tagName()  ) )
		createObjectImpl( n, objName, "self" );
	}
    }

    // database support
    dbConnections = unique( dbConnections );
    if ( dbConnections.count() )
	out << endl;
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty() && (*it) != "(default)") {
	    out << indent << (*it) << "Connection = Qt::SqlDatabase.database(\"" <<(*it) << "\");" << endl;
	}
    }

    nl = e.parentNode().toElement().elementsByTagName( "widget" );
    for ( i = 1; i < (int) nl.length(); i++ ) { // start at 1, 0 is the toplevel widget
	n = nl.item(i).toElement();
	QString s = getClassName( n );
	if ( s == "Qt::DataBrowser" || s == "Qt::DataView" ) {
	    QString objName = getObjectName( n );
	    QString tab = getDatabaseInfo( n, "table" );
	    QString con = getDatabaseInfo( n, "connection" );
	    out << indent << objName << "Form = Qt::SqlForm.new(self, \"" << objName << "Form\")" << endl;
	    QDomElement n2;
	    for ( n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() )
		createFormImpl( n2, objName, con, tab );
	    out << indent << objName << ".setForm(" << objName << "Form)" << endl;
	}
    }

    // actions, toolbars, menubar
    bool needEndl = FALSE;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "actions" ) {
	    if ( !needEndl )
		out << endl;
	    createActionImpl( n.firstChild().toElement(), "self" );
	    needEndl = TRUE;
	}
    }
    if ( needEndl )
	out << endl;
    needEndl = FALSE;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "toolbars" ) {
	    if ( !needEndl )
		out << endl;
	    createToolbarImpl( n, objClass, objName );
	    needEndl = TRUE;
	}
    }
    if ( needEndl )
	out << endl;
    needEndl = FALSE;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "menubar" ) {
	    if ( !needEndl )
		out << endl;
	    createMenuBarImpl( n, objClass, objName );
	    needEndl = TRUE;
	}
    }
    if ( needEndl )
	out << endl;

    out << indent << "languageChange()" << endl;
    
    // take minimumSizeHint() into account, for height-for-width widgets
    if ( !geometry.isNull() ) {
	out << indent << "resize( Qt::Size.new(" << geometry.width() << ", "
	    << geometry.height() << ").expandedTo(minimumSizeHint()) )" << endl;
	out << indent << "clearWState( WState_Polished )" << endl;
    }
	
	for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "connections" ) {
	    // setup signals and slots connections
	    out << endl;
	    nl = n.elementsByTagName( "connection" );
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		QString sender, receiver, signal, slot;
		for ( QDomElement n2 = nl.item(i).firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
		    if ( n2.tagName() == "sender" )
			sender = n2.firstChild().toText().data();
		    else if ( n2.tagName() == "receiver" )
			receiver = n2.firstChild().toText().data();
		    else if ( n2.tagName() == "signal" )
			signal = n2.firstChild().toText().data();
		    else if ( n2.tagName() == "slot" )
			slot = n2.firstChild().toText().data();
		}
		if ( sender.isEmpty() || receiver.isEmpty() || signal.isEmpty() || slot.isEmpty() )
		    continue;
                else if ( sender[0] == '<' ||
                    receiver[0] == '<' ||
                    signal[0] == '<' ||
                    slot[0] == '<' )
                    continue;
		sender = registeredName( sender );
		receiver = registeredName( receiver );

		if ( sender == objName )
		    sender = "self";
		if ( receiver == objName )
		    receiver = "self";

		out << indent << "Qt::Object.connect(" << sender
		    << ", SIGNAL(\"" << signal << "\"), "<< receiver << ", SLOT(\"" << slot << "\") )" << endl;
	    }
	} else if ( n.tagName()  == "tabstops" ) {
	    // setup tab order
	    out << endl;
	    QString lastName;
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "tabstop" ) {
		    QString name = n2.firstChild().toText().data();
		    name = registeredName( name );
		    if ( !lastName.isEmpty() )
			out << indent << "setTabOrder(" << lastName << ", " << name << ")" << endl;
		    lastName = name;
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
    }

// QtRuby - FIXME: what the heck is this ?
    // buddies
    bool firstBuddy = TRUE;
    for ( QValueList<Buddy>::Iterator buddy = buddies.begin(); buddy != buddies.end(); ++buddy ) {
	if ( isObjectRegistered( (*buddy).buddy ) ) {
	    if ( firstBuddy ) {
		out << endl;
	    }
	    out << indent << (*buddy).key << ".setBuddy(" << registeredName( (*buddy).buddy ) << ")" << endl;
	    firstBuddy = FALSE;
	}

    }
    if ( extraSlots.find( "init()" ) != extraSlots.end() ||
         extraFunctions.find( "init()" ) != extraFunctions.end())
        out << endl << indent << "init()" << endl;

    // end of constructor
    --indent;
    out << indent << "end" << endl;
    out << endl;



    // handle application events if required
    bool needFontEventHandler = FALSE;
    bool needSqlTableEventHandler = FALSE;
    bool needSqlDataBrowserEventHandler = FALSE;
    nl = e.elementsByTagName( "widget" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	if ( !DomTool::propertiesOfType( nl.item(i).toElement() , "font" ).isEmpty() )
	    needFontEventHandler = TRUE;
	QString s = getClassName( nl.item(i).toElement() );
	if ( s == "Qt::DataTable" || s == "Qt::DataBrowser" ) {
	    if ( !isFrameworkCodeGenerated( nl.item(i).toElement() ) )
		 continue;
	    if ( s == "Qt::DataTable" )
		needSqlTableEventHandler = TRUE;
	    if ( s == "Qt::DataBrowser" )
		needSqlDataBrowserEventHandler = TRUE;
	}
	if ( needFontEventHandler && needSqlTableEventHandler && needSqlDataBrowserEventHandler )
	    break;
    }

// PerlQt - TODO: is this needed ?
// Seems not.. let's ifzero for now...

    if ( 0 && needFontEventHandler) {
	//	indent = "\t"; // increase indentation for if-clause below
	out << endl;
	out << "#  Main event handler. Reimplemented to handle" << endl;
	out << "#  application font changes" << endl;
	out << endl;
	out << "def event( ev )" << endl;
	out << "    ret = super( ev ) " << endl;
	if ( needFontEventHandler ) {
	    ++indent;
	    out << "    if ev.type() == Qt::Event::ApplicationFontChange " << endl;
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		n = nl.item(i).toElement();
		QStringList list = DomTool::propertiesOfType( n, "font" );
		for ( it = list.begin(); it != list.end(); ++it )
		    createExclusiveProperty( n, *it );
	    }
	    out << "    end" << endl;
	    --indent;
	}
	out << "end" << endl;
	out << endl;
    }

    if ( needSqlTableEventHandler || needSqlDataBrowserEventHandler ) {
	out << endl;
	out << indent << "# Widget polish.  Reimplemented to handle default data" << endl;
	if ( needSqlTableEventHandler )
	    out << indent << "# table initialization." << endl;
	if ( needSqlDataBrowserEventHandler )
	    out << indent << "# browser initialization." << endl;
	out << indent << "def polish" << endl;
	++indent;
	if ( needSqlTableEventHandler ) {
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		QString s = getClassName( nl.item(i).toElement() );
		if ( s == "Qt::DataTable" ) {
		    n = nl.item(i).toElement();
		    QString c = getObjectName( n );
		    QString conn = getDatabaseInfo( n, "connection" );
		    QString tab = getDatabaseInfo( n, "table" );
		    if ( !( conn.isEmpty() || tab.isEmpty() ) ) {
			out << indent << "if " << c << "" << endl;
			++indent;
			out << indent << "cursor = " << c << ".sqlCursor()" << endl;
			out << endl;
			out << indent << "if ! cursor.nil?" << endl;
			++indent;
			if ( conn == "(default)" )
			    out << indent << "cursor = Qt::SqlCursor.new(\"" << tab << "\")" << endl;
			else
			    out << indent << "cursor = Qt::SqlCursor.new(\"" << tab << "\", true, " << conn << "Connection)" << endl;
			out << indent << indent << indent << "if " << c << ".isReadOnly() " << endl;
			out << indent << indent << indent << indent << "cursor.setMode( Qt::SqlCursor::ReadOnly )" << endl;
			out << indent << indent << indent << "end " << endl;
			out << indent << c << ".setSqlCursor(cursor, false, true)" << endl;
			--indent;
			out << endl;
			out << indent << "end" << endl;
			out << indent << "if !cursor.isActive()" << endl;
			++indent;
			out << indent << c << ".refresh(Qt::DataTable::RefreshAll)" << endl;
			--indent;
			out << indent << "end" << endl;
			--indent;
			out << indent << "end" << endl;
		    }
		}
	    }
	}
	if ( needSqlDataBrowserEventHandler ) {
	    nl = e.elementsByTagName( "widget" );
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		QString s = getClassName( nl.item(i).toElement() );
		if ( s == "Qt::DataBrowser" ) {
		    QString obj = getObjectName( nl.item(i).toElement() );
		    QString tab = getDatabaseInfo( nl.item(i).toElement(),
						   "table" );
		    QString conn = getDatabaseInfo( nl.item(i).toElement(),
						    "connection" );
		    if ( !(tab).isEmpty() ) {
			out << indent << "if " << obj << endl;
			++indent;
			out << indent << "if !" << obj << ".sqlCursor()" << endl;
			++indent;
			if ( conn == "(default)" )
			    out << indent << "cursor = Qt::SqlCursor.new(\"" << tab << "\");" << endl;
			else
			    out << indent << "cursor = Qt::SqlCursor.new(\"" << tab << "\", true, " << conn << "Connection)" << endl;
			out << indent << obj << ".setSqlCursor(cursor, true)" << endl;
			out << indent << obj << ".refresh()" << endl;
			out << indent << obj << ".first()" << endl;
			--indent;
			out << indent << "end" << endl;
			--indent;
			out << indent << "end" << endl;
		    }
		}
	    }
	}
	out << indent << "super()" << endl;
	--indent;
	out << indent << "end" << endl;
    }
	
    out << indent << "#" << endl;
    out << indent << "#  Sets the strings of the subwidgets using the current" << endl;
    out << indent << "#  language." << endl;
    out << indent << "#" << endl;
    out << indent << "def " << "languageChange()" << endl;
    out << languageChangeBody;
    out << indent << "end" << endl;
	out << indent << "protected :languageChange" << endl;
    out << endl;
    
	if ( !extraSlots.isEmpty() && writeSlotImpl ) {
	for ( it = extraSlots.begin(); it != extraSlots.end(); ++it ) {
	    out << endl;
	    int astart = (*it).find('(');
	    out << indent << "def " << (*it).left(astart) << "(*k)" << endl;
	    bool createWarning = TRUE;
	    QString fname = Parser::cleanArgs( *it );
	    QMap<QString, QString>::Iterator fit = functionImpls.find( fname );
	    if ( fit != functionImpls.end() ) {
		int begin = (*fit).find( "{" );
		QString body = (*fit).mid( begin + 1, (*fit).findRev( "}" ) - begin - 1 );
		createWarning = body.simplifyWhiteSpace().isEmpty();
		if ( !createWarning )
		    out << body << endl;
	    }
	    if ( createWarning ) {
		++indent;
		if ( *it != "init()" && *it != "destroy()" )
		    out << indent << "print(\"" << nameOfClass << "." << (*it) << ": Not implemented yet.\\n\")" << endl;
		--indent;
	    }
	    out << indent << "end" << endl;

	}
    }

    if ( !extraFunctions.isEmpty() ) {
	for ( it = extraFunctions.begin(); it != extraFunctions.end(); ++it ) {
	    out << endl;
	    int astart = (*it).find('(');
	    out << indent << "def " << (*it).left(astart) << "(*k)" << endl;
	    QString fname = Parser::cleanArgs( *it );
	    QMap<QString, QString>::Iterator fit = functionImpls.find( fname );
	    if ( fit != functionImpls.end() ) {
		int begin = (*fit).find( "{" );
		QString body = (*fit).mid( begin + 1, (*fit).findRev( "}" ) - begin - 1 );
		body.simplifyWhiteSpace().isEmpty();
		out << body << endl;
	    }
	    out << indent << "end" << endl;

	}
    }


    out << endl;
    out << "end" << endl;
}


/*! Creates form support implementation code for the widgets given
  in \a e.

  Traverses recursively over all children.
 */

void Uic::createFormImpl( const QDomElement& e, const QString& form, const QString& connection, const QString& table )
{
    if ( e.tagName() == "widget" &&
	 e.attribute( "class" ) != "Qt::DataTable" ) {
	QString field = getDatabaseInfo( e, "field" );
	if ( !field.isEmpty() ) {
	    if ( isWidgetInTable( e, connection, table ) )
		out << indent << form << "Form.insert(" << getObjectName( e ) << ", " << fixString( field ) << ")" << endl;
	}
    }
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	createFormImpl( n, form, connection, table );
    }
}


// Generate a QtRuby signal/slot definition.

void Uic::rubySlot(QStringList::Iterator &it)
{
    out << indent << "'" << (*it) << "'";
}
