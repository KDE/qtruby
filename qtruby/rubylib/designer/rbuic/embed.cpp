/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
** Copyright (c) 2001 Phil Thompson <phil@river-bank.demon.co.uk>
** Copyright (c) 2002 Riverbank Computing Limited <info@riverbankcomputing.co.uk>
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
#include <qfile.h>
#include <qimage.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

struct EmbedImage
{
    int width, height, depth;
    int numColors;
    QRgb* colorTable;
    QString name;
    QString cname;
    bool alpha;
};

static QString convertToCIdentifier( const char *s )
{
    QString r = s;
    int len = r.length();
    if ( len > 0 && !isalpha( (char)r[0].latin1() ) )
	r[0] = '_';
    for ( int i=1; i<len; i++ ) {
	if ( !isalnum( (char)r[i].latin1() ) )
	    r[i] = '_';
    }
    return r;
}


static ulong embedData( QTextStream& out, const uchar* input, int nbytes )
{
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
    QByteArray bazip( qCompress( input, nbytes ) );
    ulong len = bazip.size();
#else
    ulong len = nbytes;
#endif
    static const char hexdigits[] = "0123456789abcdef";
    QString s;
    for ( int i=0; i<(int)len; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    out << (const char*)s;
	    s.truncate( 0 );
	}
	uint v = (uchar)
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
		 bazip
#else
		 input
#endif
		 [i];
	s += "0x";
	s += hexdigits[(v >> 4) & 15];
	s += hexdigits[v & 15];
	if ( i < (int)len-1 )
	    s += ',';
    }
    if ( s.length() )
	out << (const char*)s;
    return len;
}

static void embedData( QTextStream& out, const QRgb* input, int n )
{
    out << hex;
    const QRgb *v = input;
    for ( int i=0; i<n; i++ ) {
	if ( (i%14) == 0  )
	    out << "\n    ";
	out << "0x";
	out << hex << *v++;
	if ( i < n-1 )
	    out << ',';
    }
    out << dec; // back to decimal mode
}

void Uic::embed( QTextStream& out, const char* project, const QStringList& images )
{

    QString cProject = convertToCIdentifier( project );

    QStringList::ConstIterator it;
    out << "# Image collection for project '" << project << "'." << endl;
    out << "#" << endl;
    out << "# Generated from reading image files: " << endl;
    for ( it = images.begin(); it != images.end(); ++it )
	out << "#      " << *it << endl;
    out << "#" << endl;
    out << "# Created: " << QDateTime::currentDateTime().toString() << endl;
    out << "#      by: The QtRuby User Interface Compiler (rbuic)" << endl;
    out << "#" << endl;
    out << "# WARNING! All changes made in this file will be lost!" << endl;
    out << endl;
    out << "require 'Qt'" << endl;
    out << endl;

    out << indent << "class MimeSourceFactory_" << cProject << " < Qt::MimeSourceFactory" << endl;
    out << endl;

    QPtrList<EmbedImage> list_image;
    int image_count = 0;
    for ( it = images.begin(); it != images.end(); ++it ) {
	QImage img;
	if ( !img.load( *it  ) ) {
	    fprintf( stderr, "rbuic: cannot load image file %s\n", (*it).latin1() );
	    continue;
	}
	EmbedImage *e = new EmbedImage;
	e->width = img.width();
	e->height = img.height();
	e->depth = img.depth();
	e->numColors = img.numColors();
	e->colorTable = new QRgb[e->numColors];
	e->alpha = img.hasAlphaBuffer();
	memcpy(e->colorTable, img.colorTable(), e->numColors*sizeof(QRgb));
	QFileInfo fi( *it );
	e->name = fi.fileName();
	e->cname = QString("$image_%1").arg( image_count++);
	list_image.append( e );
	out << "# " << *it << endl;
        QString imgname = (const char *)e->cname;

	QString s;
	if ( e->depth == 1 )
	    img = img.convertBitOrder(QImage::BigEndian);
	    out << indent << imgname << "_data = [";
	    embedData( out, img.bits(), img.numBytes() );
	out << "]\n\n";
	if ( e->numColors ) {
	    out << indent << imgname << "_ctable = [";
	    embedData( out, e->colorTable, e->numColors );
	    out << "]\n\n";
	}
    }

	++indent;
 	if ( !list_image.isEmpty() ) {
         out << indent << "@@embed_images = {\n";
	++indent;
	EmbedImage *e = list_image.first();
	while ( e )
        {
	    out << indent << "\"" << e->name << "\"" << " => [" << e->cname << "_data, "
                << e->width << ", " << e->height << ", " << e->depth << ", "
                << (e->numColors ? e->cname + "_ctable" : "[]" ) << ", "
                << (e->alpha ? "true" : "false") << "]," << endl;
	    e = list_image.next();
	}
	--indent;
	out << indent << "}" << endl;

	out << endl;
	out << indent << "@@images = Hash.new" << endl;
	out << endl;
	out << indent << "def uic_findImage( name )" << endl;
	++indent;
	out << indent << "if !@@images[name].nil?" << endl;
	++indent;
	out << indent << "return @@images[name]" << endl;
	--indent;
	out << indent << "end" << endl;
	
    out << indent << "if @@embed_images[name].nil?" << endl;
	++indent;
    out << indent << "return Qt::Image.new()" << endl;
	--indent;
    out << indent << "end" << endl;
    out << indent << endl;
#ifndef QT_NO_IMAGE_COLLECTION_COMPRESSION
	out << indent << "baunzip = qUncompress( @@embed_images[name][0].pack(\"C*\")," << endl;
	out << indent << "                       @@embed_images[name][0].length )" << endl;
	out << indent << "img = Qt::Image.new( baunzip," << endl;
	out << indent << "                     @@embed_images[name][1]," << endl;
	out << indent << "                     @@embed_images[name][2]," << endl;
	out << indent << "                     @@embed_images[name][3]," << endl;
	out << indent << "                     @@embed_images[name][4]," << endl;
	out << indent << "                     0," << endl;
	out << indent << "                     Qt::Image::BigEndian )" << endl;
#else
	out << indent << "img = Qt::Image.new( @@embed_images[name][0].pack(\"C*\")," << endl;
	out << indent << "                     @@embed_images[name][1]," << endl;
	out << indent << "                     @@embed_images[name][2]," << endl;
	out << indent << "                     @@embed_images[name][3]," << endl;
	out << indent << "                     @@embed_images[name][4]," << endl;
	out << indent << "                     0," << endl;
	out << indent << "                     Qt::Image::BigEndian )" << endl;
#endif
    out << indent << "if @@embed_images[name][5]" << endl;
	++indent;
    out << indent << "img.setAlphaBuffer(true)" << endl;
	--indent;
	out << indent << "end" << endl;
	out << indent << "@@images[name] = img" << endl;
	out << indent << "return img" << endl;
	--indent;
	out << indent << "end" << endl;
	out << endl;
	out << indent << "def data( abs_name )" << endl;
	++indent;
 	out << indent << "img = uic_findImage(abs_name)" << endl;
	out << indent << "if img.nil?" << endl;
	++indent;
	out << indent << "Qt::MimeSourceFactory.removeFactory(self)" << endl;
	out << indent << "s = Qt::MimeSourceFactory.defaultFactory().data(abs_name);" << endl;
	out << indent << "Qt::MimeSourceFactory.addFactory(self)" << endl;
	out << indent << "return s" << endl;
	--indent;
	out << indent << "end" << endl;
	out << indent << "Qt::MimeSourceFactory.defaultFactory().setImage(abs_name, img)" << endl;
	out << indent << "return Qt::MimeSourceFactory.defaultFactory().data(abs_name)" << endl;
	--indent;
	out << indent << "end" << endl;
	--indent;
    out << indent << "end" << endl;

	out << endl;
	out << endl;

    out << indent << "module StaticInitImages_" << cProject << endl;
        ++indent;
        out << indent << "@@factories = Hash.new" << endl;
        out << indent << endl;
        out << indent << "def StaticInitImages_" << cProject << ".qInitImages" << endl;
        ++indent;
        out << indent << "factory = MimeSourceFactory_" << cProject << ".new()" << endl;
        out << indent << "Qt::MimeSourceFactory.defaultFactory().addFactory(factory)" << endl;
        out << indent << "@@factories['MimeSourceFactory_" << cProject << "'] = factory" << endl;
        --indent;
        out << indent << "end" << endl;
        out << endl;
        out << indent << "def StaticInitImages_" << cProject << ".qCleanupImages" << endl;
        ++indent;
        out << indent << "for values in @@factories" << endl;
        ++indent;
        out << indent << "Qt::MimeSourceFactory.defaultFactory().removeFactory(values)" << endl;
        --indent;
        out << indent << "end" << endl;
        out << indent << "@@factories = nil" << endl;
        --indent;
        out << indent << "end" << endl;
        out << endl;
        out << indent << "StaticInitImages_" << cProject << ".qInitImages" << endl;
        --indent;
        out << indent << "end" << endl;
        out << endl;
    }
}
