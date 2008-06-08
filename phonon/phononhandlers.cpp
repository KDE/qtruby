/***************************************************************************
                          phononhandlers.cpp  -  Phonon specific marshallers
                             -------------------
    begin                : Sat Jun 28 2008
    copyright            : (C) 2008 by Richard Dale
    email                : richard.j.dale@gmail.com
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

#include <phonon/effectparameter.h>
#include <phonon/mediasource.h>
#include <phonon/path.h>

DEF_LIST_MARSHALLER( PhononEffectList, QList<Phonon::Effect*>, Phonon::Effect )

DEF_VALUELIST_MARSHALLER( PhononAudioChannelDescriptionList, QList<Phonon::AudioChannelDescription>, Phonon::AudioChannelDescription )
DEF_VALUELIST_MARSHALLER( PhononEffectParameterList, QList<Phonon::EffectParameter>, Phonon::EffectParameter )
DEF_VALUELIST_MARSHALLER( PhononMediaSourceList, QList<Phonon::MediaSource>, Phonon::MediaSource )
DEF_VALUELIST_MARSHALLER( PhononPathList, QList<Phonon::Path>, Phonon::Path )
DEF_VALUELIST_MARSHALLER( PhononSubtitleDescriptionList, QList<Phonon::SubtitleDescription>, Phonon::SubtitleDescription )

TypeHandler Phonon_handlers[] = {
    { "QList<Phonon::AudioChannelDescription>", marshall_PhononAudioChannelDescriptionList },
    { "QList<Phonon::Effect*>", marshall_PhononEffectList },
    { "QList<Phonon::EffectParameter>", marshall_PhononEffectParameterList },
    { "QList<Phonon::MediaSource>", marshall_PhononMediaSourceList },
    { "QList<Phonon::MediaSource>&", marshall_PhononMediaSourceList },
    { "QList<Phonon::Path>", marshall_PhononPathList },
    { "QList<Phonon::SubtitleDescription>", marshall_PhononSubtitleDescriptionList },
//  "QList<QExplicitlySharedDataPointer<Phonon::ObjectDescriptionData> >"
//  "QList<QExplicitlySharedDataPointer<Phonon::ObjectDescriptionData> >&"
    { 0, 0 }
};
