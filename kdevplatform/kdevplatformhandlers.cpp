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
#include <marshall_basetypes.h>

/*
"KDevelop::DUContext::DeclarationList&",
"KDevelop::DUContext::SearchItem::PtrList",
"KDevelop::DUContext::SearchItem::PtrList&",
"KDevelop::EnvironmentGroupList",
"KDevelop::EnvironmentGroupList*",
"KDevelop::EnvironmentGroupList&",
"KDevelop::Profile::EntryList",
"KDevelop::Profile::EntryList&",
"KDevelop::Profile::List",
"KDevelop::ProfileListing*",
"KDevelop::ProfileListing&",
"KDevelop::ProfileListingEx*",
"KDevelop::ProfileListingEx&",
"KPluginInfo::List",
"QList<KDevelop::AbstractType::Ptr>&",
"QList<KDevelop::Declaration*>",
"QList<KDevelop::DocumentRange*>&",
"QList<KDevelop::DUContext*>",
"QList<KDevelop::DUContext*>&",
"QList<KDevelop::IDocument*>",
"QList<KDevelop::ILanguage*>",
"QList<KDevelop::IPlugin*>",
"QList<KDevelop::IProject*>",
"QList<KDevelop::ProblemPointer>",
"QList<KDevelop::Profile*>",
"QList<KDevelop::ProjectBaseItem*>",
"QList<KDevelop::ProjectFileItem*>",
"QList<KDevelop::ProjectFolderItem*>",
"QList<KDevelop::ProjectTargetItem*>",
"QList<KDevelop::QuickOpenDataPointer>",
"QList<KDevelop::TopDUContext*>",
"QList<KDevelop::TopDUContext*>&",
"QList<KDevelop::TypeIdentifier>&",
"QList<KParts::Part*>",
"QList<KTextEditor::Document*>&",
"QList<KTextEditor::SmartRange*>",
"QList<KTextEditor::View*>&",
"QList<KUrl>",
"QList<QAction*>",
"QList<QPair<KDevelop::Declaration*,int>
"QList<QPair<KDevelop::TopDUContext*,KDevelop::SimpleCursor>
"QList<QString>&",
"QList<QVariant>",
"QList<Sublime::Area*>&",
"QList<Sublime::Document*>&",
"QList<Sublime::MainWindow*>&",
"QList<Sublime::View*>",
"QList<Sublime::View*>&",
"QMap<KDevelop::HashedString,QList<KDevelop::SimpleRange>
*/

TypeHandler KDevPlatform_handlers[] = {
    { 0, 0 }

};
