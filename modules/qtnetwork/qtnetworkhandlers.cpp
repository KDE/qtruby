/*
 *   Copyright 2009 by Richard Dale <richard.j.dale@gmail.com>

 *   Based on the PerlQt marshalling code by Ashley Winters

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

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkAddressEntry>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslCipher>
#include <QtNetwork/QSslError>

#include "marshall.h"
#include "global.h"
#include "rubymetatype.h"

Q_DECLARE_METATYPE2(QList<QPair<QHostAddress,int> >)
Q_DECLARE_METATYPE(QHostAddress)
Q_DECLARE_METATYPE(QNetworkAddressEntry)
Q_DECLARE_METATYPE(QNetworkInterface)
Q_DECLARE_METATYPE(QSslCipher)
Q_DECLARE_METATYPE(QSslError)

Q_DECLARE_METATYPE(QList<QHostAddress>)
Q_DECLARE_METATYPE(QList<QNetworkAddressEntry>)
Q_DECLARE_METATYPE(QList<QNetworkInterface>)
Q_DECLARE_METATYPE(QList<QNetworkProxy>)
Q_DECLARE_METATYPE(QList<QSslCertificate>)
Q_DECLARE_METATYPE(QList<QSslCipher>)

namespace QtRuby {


Marshall::TypeHandler QtNetworkHandlers[] = {
    { "QList<QPair<QHostAddress,int>>", marshall_Container<QList<QPair<QHostAddress,int> > > },
    { "QList<QHostAddress>", marshall_Container<QList<QHostAddress> > },
    { "QList<QHostAddress>", marshall_Container<QList<QHostAddress> > },
    { "QList<QHostAddress>&", marshall_Container<QList<QHostAddress> > },
    { "QList<QNetworkAddressEntry>&", marshall_Container<QList<QNetworkAddressEntry> > },
    { "QList<QNetworkCookie>", marshall_Container<QList<QNetworkCookie> > },
    { "QList<QNetworkCookie>&", marshall_Container<QList<QNetworkCookie> > },
    { "QList<QNetworkInterface>&", marshall_Container<QList<QNetworkInterface> > },
    { "QList<QNetworkProxy>&", marshall_Container<QList<QNetworkProxy> > },
    { "QList<QSslCertificate>&", marshall_Container<QList<QSslCertificate> > },
    { "QList<QSslCipher>&", marshall_Container<QList<QSslCipher> > },
    { "QList<QSslError>&", marshall_Container<QList<QSslError> > },

    { 0, 0 }
};

void registerQtNetworkTypes()
{
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<QHostAddress,int> > >();
    qRubySmokeRegisterSequenceMetaType<QList<QHostAddress> >();
    qRubySmokeRegisterSequenceMetaType<QList<QNetworkAddressEntry> >();
    qRubySmokeRegisterSequenceMetaType<QList<QNetworkCookie> >();
    qRubySmokeRegisterSequenceMetaType<QList<QNetworkInterface> >();
    qRubySmokeRegisterSequenceMetaType<QList<QNetworkProxy> >();
    qRubySmokeRegisterSequenceMetaType<QList<QSslCertificate> >();
    qRubySmokeRegisterSequenceMetaType<QList<QSslCipher> >();
    qRubySmokeRegisterSequenceMetaType<QList<QSslError> >();

    return;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
