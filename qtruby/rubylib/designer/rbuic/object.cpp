/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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
#include <qregexp.h>
#include <qsizepolicy.h>
#include <qstringlist.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <zlib.h>

/*!
  Creates a declaration for the object given in \a e.

  Children are not traversed recursively.

  \sa createObjectImpl()
 */
void Uic::createObjectDecl( const QDomElement& e )
{
    if ( e.tagName() == "vbox" || e.tagName() == "hbox" || e.tagName() == "grid" ) {
	out << indent << registerObject(getLayoutName(e) ) << endl;
    } else {
	QString objClass = getClassName( e );
	if ( objClass.isEmpty() )
	    return;
	QString objName = getObjectName( e );
	if ( objName.isEmpty() )
	    return;
	// ignore QLayoutWidgets
	if ( objClass == "Qt::LayoutWidget" )
	    return;

        // register the object and unify its name
	objName = registerObject( objName );
	out << indent << objName << endl;
    }
}

/*!
  Creates a PerlQt attribute declaration for the object given in \a e.

  Children are not traversed recursively.

 */
void Uic::createAttrDecl( const QDomElement& e )
{
    if ( e.tagName() == "vbox" || e.tagName() == "hbox" || e.tagName() == "grid" ) {
	out << indent << registerObject(getLayoutName(e) ) << endl;
    } else {
	QString objClass = getClassName( e );
	if ( objClass.isEmpty() )
	    return;
	QString objName = getObjectName( e );
	if ( objName.isEmpty() )
	    return;
	// ignore QLayoutWidgets
	if ( objClass == "Qt::LayoutWidget" )
	    return;
        // register the object and unify its name
	objName = registerObject( objName );
	out << indent << objName << endl;
        QDomElement n = getObjectProperty( e, "font");
	if ( !n.isNull() )
            out << indent << objName + "_font" << endl;
    }
}


/*!
  Creates an implementation for the object given in \a e.

  Traverses recursively over all children.

  Returns the name of the generated child object.

  \sa createObjectDecl()
 */

static bool createdCentralWidget = FALSE;

QString Uic::createObjectImpl( const QDomElement &e, const QString& parentClass, const QString& par, const QString& layout )
{
    QString parent( par );
    if ( parent == "self" && isMainWindow ) {
	if ( !createdCentralWidget )
	    out << indent << "setCentralWidget(Qt::Widget.new(self, \"qt_central_widget\"))" << endl;
	createdCentralWidget = TRUE;
	parent = "centralWidget()";
    }
    QDomElement n;
    QString objClass, objName, fullObjName;
    int numItems = 0;
    int numColumns = 0;
    int numRows = 0;

    if ( layouts.contains( e.tagName() ) )
	return createLayoutImpl( e, parentClass, parent, layout );

    objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return objName;
    objName = getObjectName( e );

    QString definedName = objName;
    bool isTmpObject = objName.isEmpty() || objClass == "Qt::LayoutWidget";
    if ( isTmpObject ) {
	if ( objClass[0] == 'Q' )
	    objName = objClass.mid( 4 );
	else
	    objName = objClass.lower();
    }

    bool isLine = objClass == "Line";
    if ( isLine )
	objClass = "Qt::Frame";

    out << endl;
    if ( objClass == "Qt::LayoutWidget" ) {
	if ( layout.isEmpty() ) {
	    // register the object and unify its name
	    objName = registerObject( objName );
	    out << indent << (isTmpObject ? QString::fromLatin1("") : QString::null) << objName << " = Qt::Widget.new(" << parent << ", '" << objName << "')" << endl;
	} else {
	    // the layout widget is not necessary, hide it by creating its child in the parent
	    QString result;
	    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
		if (tags.contains( n.tagName() ) )
		    result = createObjectImpl( n, parentClass, parent, layout );
	    }
	    return result;
	}

	// Layouts don't go into the class instance dictionary.
        // FIXME PerlQt: fullObjName isn't used anymore => remove
	fullObjName = objName;
    } else if ( objClass != "Qt::ToolBar" && objClass != "Qt::MenuBar" ) {
	// register the object and unify its name
	objName = registerObject( objName );

	// Temporary objects don't go into the class instance dictionary.
	fullObjName = objName;

	out << indent  << fullObjName << " = " << createObjectInstance( objClass, parent, objName )  << endl;
    }
    else
	fullObjName = objName;

    if ( objClass == "Qt::AxWidget" ) {
	QString controlId;
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "property" && n.attribute( "name" ) == "control" ) {
		controlId = n.firstChild().toElement().text();
	    }
	}
	out << "    ";
	out << fullObjName << ".setControl(\"" << controlId << "\")" << endl;
    }
	    
	lastItem = "nil";
    // set the properties and insert items
    bool hadFrameShadow = FALSE;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    bool stdset = stdsetdef;
	    if ( n.hasAttribute( "stdset" ) )
		stdset = toBool( n.attribute( "stdset" ) );
	    QString prop = n.attribute("name");
	    QString value = setObjectProperty( objClass, objName, prop, n.firstChild().toElement(), stdset );
	    if ( value.isEmpty() )
		continue;
	    if ( prop == "name" )
		continue;
	    if ( isLine && prop == "frameShadow" )
		hadFrameShadow = TRUE;
	    if ( prop == "buddy" && value[0] == '\"' && value[(int)value.length()-1] == '\"' ) {
		buddies << Buddy( objName, value.mid(1, value.length() - 2 ) );
		continue;
	    }
	    if ( isLine && prop == "orientation" ) {
		prop = "frameShape";
		if ( value.right(10) == "Horizontal" )
		    value = "Qt::Frame::HLine";
		else
		    value = "Qt::Frame::VLine";
		if ( !hadFrameShadow ) {
		    prop = "frameStyle";
		    value += " | Qt::Frame::Sunken";
		}
	    }
	    if ( prop == "buttonGroupId" ) {
		if ( parentClass == "Qt::ButtonGroup" )
		    out << indent << parent << ".insert( " << fullObjName << "," << value << ")" << endl;
		continue;
	    }
	    if ( prop == "frameworkCode" )
		continue;
	    if ( objClass == "Qt::MultiLineEdit" &&
		 QRegExp("echoMode|hMargin|maxLength|maxLines|undoEnabled").exactMatch(prop) )
		continue;
		
	    QString call = fullObjName + ".";
		if (! call.startsWith("@")) {
			call.prepend("@");
		}
		
	    if ( stdset ) {
			call += mkStdSet( prop ) + "( ";
			call += value + " )";
	    } else {
			call += "setProperty( \"" + prop + "\", Qt::Variant.new(" ;
			call += value + " ) )";
	    }

	    if ( n.firstChild().toElement().tagName() == "string" ||
		 prop == "currentItem" ) {
		trout << indent << call << endl;
	    } else {
		out << indent << call << endl;
	    }
	} else if ( n.tagName() == "item" ) {
	    QString call;
	    QString value;

	    if ( objClass.mid( 4 ) == "ListBox" ) {
		call = createListBoxItemImpl( n, fullObjName, &value );
		if ( !call.isEmpty() ) {
		    if ( numItems == 0 )
			trout << indent << fullObjName << ".clear()" << endl;
		    trout << indent << call << endl;
		}
	    } else if ( objClass.mid( 4 ) == "ComboBox" ) {
		call = createListBoxItemImpl( n, fullObjName, &value );
		if ( !call.isEmpty() ) {
		    if ( numItems == 0 )
			trout << indent << fullObjName << ".clear()" << endl;
		    trout << indent << call << endl;
		}
	    } else if ( objClass.mid( 4 ) == "IconView" ) {
		call = createIconViewItemImpl( n, fullObjName );
		if ( !call.isEmpty() ) {
		    if ( numItems == 0 )
			trout << indent << fullObjName << ".clear()" << endl;
		    trout << indent << call << endl;
		}
	    } else if ( objClass.mid( 4 ) == "ListView" ) {
		call = createListViewItemImpl( n, fullObjName, QString::null );
		if ( !call.isEmpty() ) {
		    if ( numItems == 0 )
			trout << indent << fullObjName << ".clear()" << endl;
		    trout << call << endl;
		}
		}
	    if ( !call.isEmpty() )
		numItems++;
	} else if ( n.tagName() == "column" || n.tagName() == "row" ) {
	    QString call;
	    QString value;

	    if ( objClass.mid( 4 ) == "ListView" ) {
		call = createListViewColumnImpl( n, fullObjName, &value );
		if ( !call.isEmpty() ) {
		    out << call;
		    trout << indent << fullObjName << ".header().setLabel( "
			  << numColumns++ << ", " << value << " )\n";
		}
	    } else if ( objClass ==  "Qt::Table" || objClass == "Qt::DataTable" ) {
		bool isCols = ( n.tagName() == "column" );
		call = createTableRowColumnImpl( n, fullObjName, &value );
		if ( !call.isEmpty() ) {
		    out << call;
		    trout << indent << fullObjName << "."
			  << ( isCols ? "horizontalHeader" : "verticalHeader" )
			  << "().setLabel( "
			  << ( isCols ? numColumns++ : numRows++ )
			  << ", " << value << " )\n";
		}
	    }
	}
    }

    // create all children, some widgets have special requirements

    if ( objClass == "Qt::TabWidget" ) {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( tags.contains( n.tagName()  ) ) {
		QString page = createObjectImpl( n, objClass, fullObjName );
		QString comment;
		QString label = DomTool::readAttribute( n, "title", "", comment ).toString();
		out << indent << fullObjName << ".insertTab(" << page << ", " << trcall( label ) << ")" << endl;
		trout << indent << fullObjName << ".changeTab( " << page << ", "
		      << trcall( label, comment ) << " )" << endl;
	    }
	}
    } else if ( objClass == "Qt::WidgetStack" ) {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( tags.contains( n.tagName()  ) ) {
		QString page = createObjectImpl( n, objClass, objName );
		int id = DomTool::readAttribute( n, "id", "" ).toInt();
		out << indent << fullObjName << ".addWidget( " << page << ", " << id << " )" << endl;
	    }
	}
    } else if ( objClass == "Qt::ToolBox" ) {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
		if ( tags.contains( n.tagName()  ) ) {
		QString page = createObjectImpl( n, objClass, objName );
		QString comment;
		QString label = DomTool::readAttribute( n, "label", comment ).toString();
		out << indent << fullObjName << ".addItem( " << page << ", \"\" )" << endl;
		trout << indent << fullObjName << ".setItemLabel( " << fullObjName 
		      << ".indexOf(" << page << "), " << trcall( label, comment ) 
		      << " )" << endl;
	    }
	}
     } else if ( objClass != "Qt::ToolBar" && objClass != "Qt::MenuBar" ) { // standard widgets
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( tags.contains( n.tagName() ) )
		createObjectImpl( n, objClass, fullObjName );
	}
    }

    return fullObjName;
}



/*!
  Creates a set-call for property \a exclusiveProp of the object
  given in \a e.

  If the object does not have this property, the function does nothing.

  Exclusive properties are used to generate the implementation of
  application font or palette change handlers in createFormImpl().

 */
void Uic::createExclusiveProperty( const QDomElement & e, const QString& exclusiveProp )
{
    QDomElement n;
    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;
    QString objName = getObjectName( e );
    if ( objClass.isEmpty() )
	return;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    bool stdset = stdsetdef;
	    if ( n.hasAttribute( "stdset" ) )
		stdset = toBool( n.attribute( "stdset" ) );
	    QString prop = n.attribute("name");
	    if ( prop != exclusiveProp )
		continue;
	    QString value = setObjectProperty( objClass, objName, prop, n.firstChild().toElement(), stdset );
	    if ( value.isEmpty() )
		continue;
	    out << indent << indent << objName << ".setProperty(\"" << prop << "\", Qt::Variant.new(" << value << "))" << endl;
	}
    }
}


/*!  Attention: this function has to be in sync with
  Resource::saveProperty() and DomTool::elementToVariant. If you
  change one, change all.
 */
QString Uic::setObjectProperty( const QString& objClass, const QString& obj, const QString &prop, const QDomElement &e, bool stdset )
{
    QString v;
    if ( e.tagName() == "rect" ) {
	QDomElement n3 = e.firstChild().toElement();
	int x = 0, y = 0, w = 0, h = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "x" )
		x = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "y" )
		y = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "width" )
		w = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "height" )
		h = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "Qt::Rect.new(%1, %2, %3, %4)";
	v = v.arg(x).arg(y).arg(w).arg(h);

    } else if ( e.tagName() == "point" ) {
	QDomElement n3 = e.firstChild().toElement();
	int x = 0, y = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "x" )
		x = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "y" )
		y = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "Qt::Point.new(%1, %2)";
	v = v.arg(x).arg(y);
    } else if ( e.tagName() == "size" ) {
	QDomElement n3 = e.firstChild().toElement();
	int w = 0, h = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "width" )
		w = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "height" )
		h = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "Qt::Size.new(%1, %2)";
	v = v.arg(w).arg(h);
    } else if ( e.tagName() == "color" ) {
	QDomElement n3 = e.firstChild().toElement();
	int r= 0, g = 0, b = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "red" )
		r = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "green" )
		g = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "blue" )
		b = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "Qt::Color.new(%1, %2, %3)";
	v = v.arg(r).arg(g).arg(b);
    } else if ( e.tagName() == "font" ) {
	QDomElement n3 = e.firstChild().toElement();
	QString fontname;
	if ( !obj.isEmpty() ) {
	    fontname = obj + "_font";
	    out << indent << fontname << " = Qt::Font.new(" << obj << ".font())" << endl;
	} else {
	    fontname = registerObject( "f" );
	    out << indent << fontname << " = Qt::Font.new(font())" << endl;
	}
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "family" )
		out << indent << fontname << ".setFamily(\"" << n3.firstChild().toText().data() << "\")" << endl;
	    else if ( n3.tagName() == "pointsize" )
		out << indent << fontname << ".setPointSize(" << n3.firstChild().toText().data() << ")" << endl;
	    else if ( n3.tagName() == "bold" )
		out << indent << fontname << ".setBold(" << mkBool( n3.firstChild().toText().data() ) << ")" << endl;
	    else if ( n3.tagName() == "italic" )
		out << indent << fontname << ".setItalic(" << mkBool( n3.firstChild().toText().data() ) << ")" << endl;
	    else if ( n3.tagName() == "underline" )
		out << indent << fontname << ".setUnderline(" << mkBool( n3.firstChild().toText().data() ) << ")" << endl;
	    else if ( n3.tagName() == "strikeout" )
		out << indent << fontname << ".setStrikeOut(" << mkBool( n3.firstChild().toText().data() ) << ")" << endl;
	    n3 = n3.nextSibling().toElement();
	}

	if ( prop == "font" ) {
	    if ( !obj.isEmpty() )
		out << indent << obj << ".setFont(" << fontname << ")" << endl;
	    else
		out << indent << "setFont(" << fontname << ")" << endl;
	} else {
	    v = fontname;
	}
    } else if ( e.tagName() == "string" ) {
	QString txt = e.firstChild().toText().data();
	QString com = getComment( e.parentNode() );

	if ( prop == "toolTip" && objClass != "Qt::Action" && objClass != "Qt::ActionGroup" ) {
	    if ( !obj.isEmpty() )
		trout << indent << "Qt::ToolTip.add( " << obj << ", "
		      << trcall( txt, com ) << " )" << endl;
	    else
		trout << indent << "Qt::ToolTip.add( self, "
		      << trcall( txt, com ) << " )" << endl;
	} else if ( prop == "whatsThis" && objClass != "Qt::Action" && objClass != "Qt::ActionGroup" ) {
	    if ( !obj.isEmpty() )
		trout << indent << "Qt::WhatsThis.add(" << obj << ", " << trcall( txt, com ) << ")" << endl;
	    else
		trout << indent << "Qt::WhatsThis.add(self," << trcall( txt, com ) << ")" << endl;
	} else if (e.parentNode().toElement().attribute("name") == "accel") {
            v = "Qt::KeySequence.new(" + trcall( txt, com ) + ")";
        } else {
	    v = trcall( txt, com );
	}
    } else if ( e.tagName() == "cstring" ) {
	    v = "\"%1\"";
	    v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "number" ) {
        // FIXME: hack. QtRuby needs a QKeySequence to build an accel
	if( e.parentNode().toElement().attribute("name") == "accel" )
            v = "Qt::KeySequence.new(%1)";
	else
	    v = "%1";
	v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "bool" ) {
	if ( stdset )
	    v = "%1";
	else
	    v = "Qt::Variant.new(%1, 0)";
	v = v.arg( mkBool( e.firstChild().toText().data() ) );
    } else if ( e.tagName() == "pixmap" ) {
	v = e.firstChild().toText().data();
        if( !externPixmaps )
        	v.prepend( '@' );
	if ( !pixmapLoaderFunction.isEmpty() ) {
	    v.prepend( pixmapLoaderFunction + "(" + QString( externPixmaps ? "\"" : "" ) );
	    v.append( QString( externPixmaps ? "\"" : "" ) + ")" );
	}
    } else if ( e.tagName() == "iconset" ) {
	v = "Qt::IconSet.new(%1)";
	QString s = e.firstChild().toText().data();
	if ( !pixmapLoaderFunction.isEmpty() ) {
	    s.prepend( pixmapLoaderFunction + "(" + QString( externPixmaps ? "\"" : "" ) );
	    s.append( QString( externPixmaps ? "\"" : "" ) + ")" );
	} else {
		s.prepend("@");
	}
	v = v.arg( s );
    } else if ( e.tagName() == "image" ) {
	v = e.firstChild().toText().data() + ".convertToImage()";
    } else if ( e.tagName() == "enum" ) {
	v = "%1::%2";
	QString oc = objClass;
	QString ev = e.firstChild().toText().data();
	if ( oc == "Qt::ListView" && ev == "Manual" ) // #### workaround, rename QListView::Manual of WithMode enum in 3.0
	    oc = "Qt::ScrollView";
	v = v.arg( oc ).arg( ev );
    } else if ( e.tagName() == "set" ) {
	QString keys( e.firstChild().toText().data() );
	QStringList lst = QStringList::split( '|', keys );
	v = "";
#if defined(Q_CC_EDG)
	// workaround for EDG bug reproduced with MIPSpro C++ 7.3.?
	// and KAI C++ 4.0e that will be fixed in KAI C++ 4.0f
	QStringList::Iterator it = lst.begin();
	for ( ; it != lst.end(); ++it ) {
#else
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
#endif
	    v += objClass + "::" + *it;
	    if ( it != lst.fromLast() )
		v += " | ";
	}
        v += "";
    } else if ( e.tagName() == "sizepolicy" ) {
	QDomElement n3 = e.firstChild().toElement();
	QSizePolicy sp;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "hsizetype" )
		sp.setHorData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "vsizetype" )
		sp.setVerData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "horstretch" )
		sp.setHorStretch( n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "verstretch" )
		sp.setVerStretch( n3.firstChild().toText().data().toInt() );
	    n3 = n3.nextSibling().toElement();
	}
	QString tmp = (obj.isEmpty() ? QString::fromLatin1("self") : obj) + ".";
	v = "Qt::SizePolicy.new(%1, %2, %3, %4, " + tmp + "sizePolicy().hasHeightForWidth())";
	v = v.arg( (int)sp.horData() ).arg( (int)sp.verData() ).arg( sp.horStretch() ).arg( sp.verStretch() );
    } else if ( e.tagName() == "palette" ) {
	QPalette pal;
	bool no_pixmaps = e.elementsByTagName( "pixmap" ).count() == 0;
	QDomElement n;
	if ( no_pixmaps ) {
	    n = e.firstChild().toElement();
	    while ( !n.isNull() ) {
		QColorGroup cg;
		if ( n.tagName() == "active" ) {
		    cg = loadColorGroup( n );
		    pal.setActive( cg );
		} else if ( n.tagName() == "inactive" ) {
		    cg = loadColorGroup( n );
		    pal.setInactive( cg );
		} else if ( n.tagName() == "disabled" ) {
		    cg = loadColorGroup( n );
		    pal.setDisabled( cg );
		}
		n = n.nextSibling().toElement();
	    }
	}
	if ( no_pixmaps && pal == QPalette( pal.active().button(), pal.active().background() ) ) {
	    v = "Qt::Palette.new(Qt::Color.new(%1,%2,%3), Qt::Color.new(%1,%2,%3))";
	    v = v.arg( pal.active().button().red() ).arg( pal.active().button().green() ).arg( pal.active().button().blue() );
	    v = v.arg( pal.active().background().red() ).arg( pal.active().background().green() ).arg( pal.active().background().blue() );
	} else {
	    QString palette = "pal";
	    if ( !pal_used ) {
		out << indent << palette << " = Qt::Palette.new()" << endl;
		pal_used = TRUE;
	    }
	    QString cg = "cg";
	    if ( !cg_used ) {
		out << indent << cg << " = Qt::ColorGroup.new()" << endl;
		cg_used = TRUE;
	    }
	    n = e.firstChild().toElement();
	    while ( !n.isNull() && n.tagName() != "active" )
		n = n.nextSibling().toElement();
	    createColorGroupImpl( cg, n );
	    out << indent << palette << ".setActive(" << cg << ")" << endl;

	    n = e.firstChild().toElement();
	    while ( !n.isNull() && n.tagName() != "inactive" )
		n = n.nextSibling().toElement();
	    createColorGroupImpl( cg, n );
	    out << indent << palette << ".setInactive(" << cg << ")" << endl;

	    n = e.firstChild().toElement();
	    while ( !n.isNull() && n.tagName() != "disabled" )
		n = n.nextSibling().toElement();
	    createColorGroupImpl( cg, n );
	    out << indent << palette << ".setDisabled(" << cg << ")" << endl;
	    v = palette;
	}
    } else if ( e.tagName() == "cursor" ) {
	v = "Qt::Cursor.new(%1)";
	v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "date" ) {
	QDomElement n3 = e.firstChild().toElement();
	int y, m, d;
	y = m = d = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "year" )
		y = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "month" )
		m = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "day" )
		d = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "Qt::Date.new(%1,%2,%3)";
	v = v.arg(y).arg(m).arg(d);
    } else if ( e.tagName() == "time" ) {
	QDomElement n3 = e.firstChild().toElement();
	int h, m, s;
	h = m = s = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "hour" )
		h = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "minute" )
		m = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "second" )
		s = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "Qt::Time.new(%1, %2, %3)";
	v = v.arg(h).arg(m).arg(s);
    } else if ( e.tagName() == "datetime" ) {
	QDomElement n3 = e.firstChild().toElement();
	int h, mi, s, y, mo, d;
	h = mi = s = y = mo = d = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "hour" )
		h = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "minute" )
		mi = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "second" )
		s = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "year" )
		y = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "month" )
		mo = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "day" )
		d = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "Qt::DateTime.new(Qt::Date.new(%1, %2, %3), Qt::Time.new(%4, %5, %6))";
	v = v.arg(y).arg(mo).arg(d).arg(h).arg(mi).arg(s);
    } else if ( e.tagName() == "stringlist" ) {
	QStringList l;
	QDomElement n3 = e.firstChild().toElement();
	QString listname;
	if ( !obj.isEmpty() ) {
	    listname = obj + ".[_strlist";
	    listname = registerObject( listname );
            listname += "]";
	    out << indent << listname << " = [";
	} else {
            listname = "strlist";
	    out << indent << listname << " = [";
	}
        int i = 0;
	while ( true ) {
	    if ( n3.tagName() == "string" )
            {
		out << "'" << n3.firstChild().toText().data().simplifyWhiteSpace() << "'";
	        n3 = n3.nextSibling().toElement();
                i++;
		if( n3.isNull() )
                    break;
                else if( (i%3) == 0 )
                {
                    ++indent;
                    out << "," << endl << indent;
                    --indent;
                }
                else if( n3.isNull() )
                    break;
                else
                    out << ", ";
	    }
	}
        out << "]" << endl;
	v = listname;
    }
    return v;
}




/*! Extracts a named object property from \a e.
 */
QDomElement Uic::getObjectProperty( const QDomElement& e, const QString& name )
{
    QDomElement n;
    for ( n = e.firstChild().toElement();
	  !n.isNull();
	  n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property"  && n.toElement().attribute("name") == name )
	    return n;
    }
    return n;
}

