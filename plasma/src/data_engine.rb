=begin
 *   Copyright 2008 by Richard Dale <richard.j.dale@gmail.com>
 *
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
=end

require 'plasma_applet'

module RubyAppletScript
  class DataEngine < Plasma::DataEngineScript
    def initialize(parent, args)
      super(parent)
    end

    def camelize(str)
      str.gsub(/(^|[_-])(.)/) { $2.upcase }
    end

    def init
      puts "RubyAppletScript::DataEngine#init mainScript: #{mainScript}"
      program = Qt::FileInfo.new(mainScript)
      load Qt::File.encodeName(program.filePath).to_s
      moduleName = camelize(package.metadata.name)
      className = camelize(program.baseName)
      puts "RubyAppletScript::DataEngine#init instantiating: #{moduleName}::#{className}"
      klass = Object.const_get(moduleName.to_sym).const_get(className.to_sym)
      @data_engine_script = klass.new(self)
      @data_engine_script.init
      return true
    end

    def sourceRequestEvent(name)
      @data_engine.sourceRequestEvent(name)
    end

    def updateSourceEvent(source)
      @data_engine.updateSourceEvent(source)
    end
  end
end