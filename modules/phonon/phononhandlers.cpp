/*
 *   Copyright 2009 by Richard Dale <richard.j.dale@gmail.com>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "marshall.h"
#include "global.h"
#include "rubymetatype.h"

#include <phonon/effect.h>
#include <phonon/effectparameter.h>
#include <phonon/effectwidget.h>
#include <phonon/mediasource.h>
#include <phonon/path.h>
#include <phonon/volumefadereffect.h>

Q_DECLARE_METATYPE(Phonon::Effect*)
Q_DECLARE_METATYPE(Phonon::EffectParameter)
Q_DECLARE_METATYPE(Phonon::MediaSource)
// Q_DECLARE_METATYPE(Phonon::ObjectDescription<Phonon::AudioChannelType>)
// Q_DECLARE_METATYPE(Phonon::ObjectDescription<Phonon::SubtitleType>)
Q_DECLARE_METATYPE(Phonon::Path)
Q_DECLARE_METATYPE(QList<Phonon::Effect*>)
Q_DECLARE_METATYPE(QList<Phonon::EffectParameter>)
Q_DECLARE_METATYPE(QList<Phonon::MediaSource>)
Q_DECLARE_METATYPE(QList<Phonon::Path>)

namespace QtRuby {

Marshall::TypeHandler PhononHandlers[] = {
    { "QList<Phonon::Effect*>", marshall_Container<QList<Phonon::Effect*> > },
    { "QList<Phonon::EffectParameter>", marshall_Container<QList<Phonon::EffectParameter> > },
    { "QList<Phonon::MediaSource>", marshall_Container<QList<Phonon::MediaSource> > },
    { "QList<Phonon::MediaSource>&", marshall_Container<QList<Phonon::MediaSource> > },
    { "QList<Phonon::Path>", marshall_Container<QList<Phonon::Path> > },
    { 0, 0 }
};

void registerPhononTypes()
{
    qRubySmokeRegisterSequenceMetaType<QList<Phonon::EffectParameter> >();
    qRubySmokeRegisterSequenceMetaType<QList<Phonon::MediaSource> >();
    qRubySmokeRegisterSequenceMetaType<QList<Phonon::Path> >();

    qRubySmokeRegisterPointerSequenceMetaType<QList<Phonon::Effect*> >();

    return;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
