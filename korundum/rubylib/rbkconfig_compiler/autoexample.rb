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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
=end

require 'Korundum'
require 'exampleprefs_base.rb'
require 'general_base.rb'
require 'myoptions_base.rb'

  aboutData = KDE::AboutData.new( "example", I18N_NOOP("autoconfig example"), "0.1" )
  aboutData.addAuthor( "Cornelius Schumacher", nil, "schumacher@kde.org" )

  KDE::CmdLineArgs.init( ARGV, aboutData )

  app = KDE::Application.new

  configSkeleton = ExamplePrefsBase.new( "dummy1", "dummy2" )
  configSkeleton.readConfig()

  dialog = KDE::ConfigDialog( nil, "settings", configSkeleton )
  
  general = GeneralBase.new(nil)
  dialog.addPage( general, i18n("General"), "General", "" )

  myOptions = MyOptionsBase.new( nil )
  dialog.addPage( myOptions, i18n("MyOptions"), "MyOptions", "" )

  app.mainWidget = dialog 

  dialog.show()
    
  app.exec
