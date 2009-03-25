/***************************************************************************
                          sopranohandlers.cpp  -  Soprano specific marshallers
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003 by Richard Dale
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

#include <soprano/soprano.h>
#include <soprano/inferencerule.h>
#include <soprano/statementpattern.h>

DEF_LIST_MARSHALLER( SopranoBackendList, QList<const Soprano::Backend*>, Soprano::Backend )
DEF_LIST_MARSHALLER( SopranoParserList, QList<const Soprano::Parser*>, Soprano::Parser )
DEF_LIST_MARSHALLER( SopranoSerializerList, QList<const Soprano::Serializer*>, Soprano::Serializer )

DEF_VALUELIST_MARSHALLER( SopranoBindingSetList, QList<Soprano::BindingSet>, Soprano::BindingSet )
DEF_VALUELIST_MARSHALLER( SopranoInferenceRuleList, QList<Soprano::Inference::Rule>, Soprano::Inference::Rule )
DEF_VALUELIST_MARSHALLER( SopranoInferenceStatementPatternList, QList<Soprano::Inference::StatementPattern>, Soprano::Inference::StatementPattern )
DEF_VALUELIST_MARSHALLER( SopranoNodeList, QList<Soprano::Node>, Soprano::Node )
DEF_VALUELIST_MARSHALLER( SopranoStatementList, QList<Soprano::Statement>, Soprano::Statement )
DEF_VALUELIST_MARSHALLER( SopranoBackendSettingList, QList<Soprano::BackendSetting>, Soprano::BackendSetting )

TypeHandler Soprano_handlers[] = {
	{ "QList<Soprano::BindingSet>", marshall_SopranoBindingSetList },
	{ "QList<Soprano::Inference::Rule>", marshall_SopranoInferenceRuleList },
	{ "QList<Soprano::Inference::StatementPattern>", marshall_SopranoInferenceStatementPatternList },
	{ "QList<Soprano::Node>", marshall_SopranoNodeList },
	{ "QList<Soprano::Statement>", marshall_SopranoStatementList  },
	{ "QList<const Soprano::Backend*>", marshall_SopranoBackendList },
	{ "QList<const Soprano::Parser*>", marshall_SopranoParserList  },
    { "QList<const Soprano::Serializer*>", marshall_SopranoBindingSetList },
    { "QList<Soprano::BackendSetting>", marshall_SopranoBackendSettingList },
    { "QList<Soprano::BackendSetting>&", marshall_SopranoBackendSettingList },
    { 0, 0 }
};

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
