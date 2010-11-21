/***************************************************************************
                          okularhandlers.cpp  -  Okular specific marshallers
                             -------------------
    begin                : Thurs May 29 2008
    copyright            : (C) 2008 by Richard Dale
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

#include <ruby.h>

#include <qtruby.h>
#include <smokeruby.h>
#include <marshall_macros.h>

#include <okular/core/annotations.h>
#include <okular/core/area.h>
#include <okular/core/document.h>
#include <okular/core/form.h>
#include <okular/core/generator.h>
#include <okular/core/page.h>
#include <okular/core/pagesize.h>
#include <okular/core/pagetransition.h>

// DEF_LINKED_VALUELIST_MARSHALLER( OkularAnnotationRevisionLinkedList, QLinkedList<Okular::Annotation::Revision>, Okular::Annotation::Revision )
DEF_LINKED_VALUELIST_MARSHALLER( OkularNormalizedPointLinkedList, QLinkedList<Okular::NormalizedPoint>, Okular::NormalizedPoint )

DEF_LINKED_LIST_MARSHALLER( OkularAnnotationLinkedList, QLinkedList<Okular::Annotation*>, Okular::Annotation )
DEF_LINKED_LIST_MARSHALLER( OkularFormFieldLinkedList, QLinkedList<Okular::FormField*>, Okular::FormField )
DEF_LINKED_LIST_MARSHALLER( OkularObjectRectLinkedList, QLinkedList<Okular::ObjectRect*>, Okular::ObjectRect )
DEF_LINKED_LIST_MARSHALLER( OkularPixmapRequestLinkedList, QLinkedList<Okular::PixmapRequest*>, Okular::PixmapRequest )
DEF_LINKED_LIST_MARSHALLER( OkularSourceRefObjectRectLinkedList, QLinkedList<Okular::SourceRefObjectRect*>, Okular::SourceRefObjectRect )

DEF_VALUELIST_MARSHALLER( OkularExportFormatList, QList<Okular::ExportFormat>, Okular::ExportFormat )
DEF_VALUELIST_MARSHALLER( OkularHighlightAnnotationQuadList, QList<Okular::HighlightAnnotation::Quad>, Okular::HighlightAnnotation::Quad )

DEF_LIST_MARSHALLER( OkularAnnotationList, QList<Okular::Annotation*>, Okular::Annotation )
DEF_LIST_MARSHALLER( OkularEmbeddedFileList, QList<Okular::EmbeddedFile*>, Okular::EmbeddedFile )
DEF_LIST_MARSHALLER( OkularPageList, QVector<Okular::Page*>, Okular::Page )
DEF_LIST_MARSHALLER( OkularVisiblePageRectList, QVector<Okular::VisiblePageRect*>, Okular::VisiblePageRect )

TypeHandler Okular_handlers[] = {
    { "QLinkedList<Okular::Annotation*>", marshall_OkularAnnotationLinkedList },
//    { "QLinkedList<Okular::Annotation::Revision>&", marshall_OkularAnnotationRevisionLinkedList },
    { "QLinkedList<Okular::FormField*>", marshall_OkularFormFieldLinkedList },
    { "QLinkedList<Okular::FormField*>&", marshall_OkularFormFieldLinkedList },
    { "QLinkedList<Okular::NormalizedPoint>", marshall_OkularNormalizedPointLinkedList },
    { "QLinkedList<Okular::NormalizedPoint>&", marshall_OkularNormalizedPointLinkedList },
    { "QLinkedList<Okular::ObjectRect*>&", marshall_OkularObjectRectLinkedList },
    { "QLinkedList<Okular::PixmapRequest*>&", marshall_OkularPixmapRequestLinkedList },
    { "QLinkedList<Okular::SourceRefObjectRect*>&", marshall_OkularSourceRefObjectRectLinkedList },
    { "QList<Okular::Annotation*>&", marshall_OkularAnnotationList },
    { "QList<Okular::EmbeddedFile*>*", marshall_OkularEmbeddedFileList },
    { "QList<Okular::ExportFormat>", marshall_OkularExportFormatList },
    { "QList<Okular::HighlightAnnotation::Quad>&", marshall_OkularHighlightAnnotationQuadList },
//    { "QList<QLinkedList<Okular::NormalizedPoint> >", marshall_QLinkedList<OkularNormalizedPoint> List },
//    { "QList<QLinkedList<Okular::NormalizedPoint> >&", marshall_QLinkedList<OkularNormalizedPoint> List },
    { "QVector<Okular::Page*>&", marshall_OkularPageList },
    { "QVector<Okular::VisiblePageRect*>&", marshall_OkularVisiblePageRectList },
    { 0, 0 }
};
