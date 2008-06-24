/***************************************************************************
                          kdevplatformhandlers.cpp  -  KDevPlatform specific marshallers
                             -------------------
    begin                : Sat Jun 7 2008
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

#include <kdevplatform/interfaces/idocument.h>
#include <kdevplatform/interfaces/ilanguage.h>
#include <kdevplatform/interfaces/iplugin.h>
#include <kdevplatform/interfaces/iproject.h>
#include <kdevplatform/language/interfaces/iproblem.h>
#include <kdevplatform/language/duchain/declaration.h>
#include <kdevplatform/language/duchain/ducontext.h>
#include <kdevplatform/language/duchain/identifier.h>
#include <kdevplatform/language/duchain/topducontext.h>
#include <kdevplatform/language/interfaces/quickopendataprovider.h>
#include <kdevplatform/project/projectmodel.h>
#include <kdevplatform/shell/language.h>
#include <kdevplatform/shell/profile.h>
#include <kdevplatform/sublime/area.h>
#include <kdevplatform/sublime/document.h>
#include <kdevplatform/sublime/mainwindow.h>
#include <kdevplatform/sublime/tooldocument.h>
#include <kdevplatform/sublime/urldocument.h>
#include <kdevplatform/sublime/view.h>

/* TODO
"KDevelop::DUContext::DeclarationList&",
"KDevelop::DUContext::SearchItem::PtrList",
"KDevelop::DUContext::SearchItem::PtrList&",
"QList<KDevelop::AbstractType::Ptr>&",
"QMap<KDevelop::HashedString,QList<KDevelop::SimpleRange>
*/

DEF_VALUELIST_MARSHALLER( KDevelopProblemPointerList, QList<KDevelop::ProblemPointer>, KDevelop::ProblemPointer )
DEF_VALUELIST_MARSHALLER( KDevelopQuickOpenDataPointerList, QList<KDevelop::QuickOpenDataPointer>, KDevelop::QuickOpenDataPointer )
DEF_VALUELIST_MARSHALLER( KDevelopTypeIdentifierList, QList<KDevelop::TypeIdentifier>, KDevelop::TypeIdentifier )
DEF_VALUELIST_MARSHALLER( KDevelopProfileEntryList, QList<KDevelop::Profile::Entry>, KDevelop::Profile::Entry )

DEF_LIST_MARSHALLER( KDevelopDUContextList, QList<KDevelop::DUContext*>, KDevelop::DUContext )
DEF_LIST_MARSHALLER( KDevelopDeclarationList, QList<KDevelop::Declaration*>, KDevelop::Declaration )
DEF_LIST_MARSHALLER( KDevelopDocumentRangeList, QList<KDevelop::DocumentRange*>, KDevelop::DocumentRange )
DEF_LIST_MARSHALLER( KDevelopIDocumentList, QList<KDevelop::IDocument*>, KDevelop::IDocument )
DEF_LIST_MARSHALLER( KDevelopILanguageList, QList<KDevelop::ILanguage*>, KDevelop::ILanguage )
DEF_LIST_MARSHALLER( KDevelopIPluginList, QList<KDevelop::IPlugin*>, KDevelop::IPlugin )
DEF_LIST_MARSHALLER( KDevelopIProjectList, QList<KDevelop::IProject*>, KDevelop::IProject )
DEF_LIST_MARSHALLER( KDevelopProfileList, QList<KDevelop::Profile*>, KDevelop::Profile )
DEF_LIST_MARSHALLER( KDevelopProjectBaseItemList, QList<KDevelop::ProjectBaseItem*>, KDevelop::ProjectBaseItem )
DEF_LIST_MARSHALLER( KDevelopProjectFileItemList, QList<KDevelop::ProjectFileItem*>, KDevelop::ProjectFileItem )
DEF_LIST_MARSHALLER( KDevelopProjectFolderItemList, QList<KDevelop::ProjectFolderItem*>, KDevelop::ProjectFolderItem )
DEF_LIST_MARSHALLER( KDevelopProjectTargetItemList, QList<KDevelop::ProjectTargetItem*>, KDevelop::ProjectTargetItem )
DEF_LIST_MARSHALLER( KDevelopTopDUContextList, QList<KDevelop::TopDUContext*>, KDevelop::TopDUContext )
DEF_LIST_MARSHALLER( KPartsPartList, QList<KParts::Part*>, KParts::Part )
DEF_LIST_MARSHALLER( KTextEditorDocumentList, QList<KTextEditor::Document*>, KTextEditor::Document )
DEF_LIST_MARSHALLER( KTextEditorSmartRangeList, QList<KTextEditor::SmartRange*>, KTextEditor::SmartRange )
DEF_LIST_MARSHALLER( KTextEditorViewList, QList<KTextEditor::View*>, KTextEditor::View )
DEF_LIST_MARSHALLER( SublimeAreaList, QList<Sublime::Area*>, Sublime::Area )
DEF_LIST_MARSHALLER( SublimeDocumentList, QList<Sublime::Document*>, Sublime::Document )
DEF_LIST_MARSHALLER( SublimeMainWindowList, QList<Sublime::MainWindow*>, Sublime::MainWindow )
DEF_LIST_MARSHALLER( SublimeViewList, QList<Sublime::View*>, Sublime::View )
DEF_LIST_MARSHALLER( KDevelopDUContextVector, QVector<KDevelop::DUContext*>, KDevelop::DUContext )
DEF_LIST_MARSHALLER( KDevelopDeclarationVector, QVector<KDevelop::Declaration*>, KDevelop::Declaration )

TypeHandler KDevPlatform_handlers[] = {
    { "QList<KDevelop::DUContext*>", marshall_KDevelopDUContextList },
    { "QList<KDevelop::DUContext*>&", marshall_KDevelopDUContextList },
    { "QList<KDevelop::Declaration*>", marshall_KDevelopDeclarationList },
    { "QList<KDevelop::DocumentRange*>&", marshall_KDevelopDocumentRangeList },
    { "QList<KDevelop::IDocument*>", marshall_KDevelopIDocumentList },
    { "QList<KDevelop::ILanguage*>", marshall_KDevelopILanguageList },
    { "QList<KDevelop::IPlugin*>", marshall_KDevelopIPluginList },
    { "QList<KDevelop::IProject*>", marshall_KDevelopIProjectList },
    { "QList<KDevelop::ProblemPointer>", marshall_KDevelopProblemPointerList },
    { "QList<KDevelop::Profile*>", marshall_KDevelopProfileList },
    { "KDevelop::Profile::EntryList", marshall_KDevelopProfileEntryList },
    { "KDevelop::Profile::EntryList&", marshall_KDevelopProfileEntryList },
    { "QList<KDevelop::ProjectBaseItem*>", marshall_KDevelopProjectBaseItemList },
    { "QList<KDevelop::ProjectFileItem*>", marshall_KDevelopProjectFileItemList },
    { "QList<KDevelop::ProjectFolderItem*>", marshall_KDevelopProjectFolderItemList },
    { "QList<KDevelop::ProjectTargetItem*>", marshall_KDevelopProjectTargetItemList },
    { "QList<KDevelop::QuickOpenDataPointer>", marshall_KDevelopQuickOpenDataPointerList },
    { "QList<KDevelop::TopDUContext*>", marshall_KDevelopTopDUContextList },
    { "QList<KDevelop::TopDUContext*>&", marshall_KDevelopTopDUContextList },
    { "QList<KDevelop::TypeIdentifier>&", marshall_KDevelopTypeIdentifierList },
    { "QList<KParts::Part*>", marshall_KPartsPartList },
    { "QList<KTextEditor::Document*>&", marshall_KTextEditorDocumentList },
    { "QList<KTextEditor::SmartRange*>", marshall_KTextEditorSmartRangeList },
    { "QList<KTextEditor::View*>&", marshall_KTextEditorViewList },
    { "QList<Sublime::Area*>&", marshall_SublimeAreaList },
    { "QList<Sublime::Document*>&", marshall_SublimeDocumentList },
    { "QList<Sublime::MainWindow*>&", marshall_SublimeMainWindowList },
    { "QList<Sublime::View*>", marshall_SublimeViewList },
    { "QList<Sublime::View*>&", marshall_SublimeViewList },
    { "QVector<KDevelop::DUContext*>&", marshall_KDevelopDUContextVector },
    { "QVector<KDevelop::Declaration*>", marshall_KDevelopDeclarationVector },
    { 0, 0 }
};
