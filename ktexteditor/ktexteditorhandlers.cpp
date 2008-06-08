/***************************************************************************
                          ktexteditorhandlers.cpp  -  KTextEditor specific marshallers
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

#include <ktexteditor/commandinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/view.h>

DEF_LIST_MARSHALLER( KTextEditorCommandList, QList<KTextEditor::Command*>, KTextEditor::Command )
DEF_LIST_MARSHALLER( KTextEditorDocumentList, QList<KTextEditor::Document*>, KTextEditor::Document )
DEF_LIST_MARSHALLER( KTextEditorSmartRangeList, QList<KTextEditor::SmartRange*>, KTextEditor::SmartRange )
DEF_LIST_MARSHALLER( KTextEditorSmartRangeNotifierList, QList<KTextEditor::SmartRangeNotifier*>, KTextEditor::SmartRangeNotifier )
DEF_LIST_MARSHALLER( KTextEditorSmartRangeWatcherList, QList<KTextEditor::SmartRangeWatcher*>, KTextEditor::SmartRangeWatcher )
DEF_LIST_MARSHALLER( KTextEditorViewList, QList<KTextEditor::View*>, KTextEditor::View )

// DEF_VALUELIST_MARSHALLER( KTextEditorAgentInstanceList, QList<KTextEditor::AgentInstance>, KTextEditor::AgentInstance )

TypeHandler KTextEditor_handlers[] = {
//    { "KTextEditor::AgentInstance::List", marshall_KTextEditorAgentInstanceList },
    { "QList<KTextEditor::Command*>", marshall_KTextEditorCommandList },
    { "QList<KTextEditor::Document*>&", marshall_KTextEditorDocumentList },
    { "QList<KTextEditor::SmartRange*>", marshall_KTextEditorSmartRangeList },
    { "QList<KTextEditor::SmartRange*>&", marshall_KTextEditorSmartRangeList },
    { "QList<KTextEditor::SmartRangeNotifier*>", marshall_KTextEditorSmartRangeNotifierList },
    { "QList<KTextEditor::SmartRangeWatcher*>&", marshall_KTextEditorSmartRangeWatcherList },
    { "QList<KTextEditor::View*>&", marshall_KTextEditorViewList },
    { 0, 0 }
};
