=begin
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
=end

require 'Korundum'
require 'exampleprefs_base.rb'

  aboutData = KDE::AboutData.new( "example", I18N_NOOP("cfgc example"), "0.1" )
  aboutData.addAuthor( "Cornelius Schumacher", nil, "schumacher@kde.org" )

  KDE::CmdLineArgs.init( ARGV, aboutData )

  app = KDE::Application.new

  prefs = ExamplePrefsBase.new("Trans1", "Folder2")
  
  prefs.readConfig()

  prefs.setAnotherOption(17)


  qWarning("Another Option = %d" % prefs.anotherOption())
  qWarning("Another Option2 = %d" % prefs.anotherOption2())
