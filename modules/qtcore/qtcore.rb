=begin
     Copyright 2009-2011 by Richard Dale <richard.j.dale@gmail.com>

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU Library General Public License as
     published by the Free Software Foundation; either version 2, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details

     You should have received a copy of the GNU Library General Public
     License along with this program; if not, write to the
     Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
=end

module Qt
  module DebugLevel
    Off, Minimal, High, Extensive = 0, 1, 2, 3
  end

  module QtDebugChannel
    QTDB_NONE = 0x00
    QTDB_METHOD_MATCHES = 0x01
    QTDB_METHOD_MISSING = 0x02
    QTDB_CALLS = 0x04
    QTDB_GC = 0x08
    QTDB_VIRTUAL = 0x10
    QTDB_VERBOSE = 0x20
    QTDB_ALL = QTDB_VERBOSE | QTDB_VIRTUAL | QTDB_GC | QTDB_CALLS | QTDB_METHOD_MISSING | QTDB_METHOD_MATCHES
  end

  @@debug_level = DebugLevel::Off
  def Qt.debug_level=(level)
    @@debug_level = level
    Internal::setDebug Qt::QtDebugChannel::QTDB_ALL if level >= DebugLevel::Extensive
  end

  def Qt.debug_level
    @@debug_level
  end
    
  module Internal

    # Runs the initializer as far as allocating the Qt C++ instance.
    # Then use a throw to jump back to here with the C++ instance
    # wrapped in a new ruby variable of type T_DATA
    def Internal.try_initialize(instance,  args)
      initializer = instance.method(:initialize)
      catch :newqt do
        initializer.call(*args)
      end
    end

    # If a block was passed to the constructor, then
    # run that now. Either run the context of the new instance
    # if no args were passed to the block. Or otherwise,
    # run the block in the context of the arg.
    def Internal.run_initializer_block(instance, block)
      if block.arity == -1 || block.arity == 0
        instance.instance_eval(&block)
      elsif block.arity == 1
        block.call(instance)
      else
        raise ArgumentError, "Wrong number of arguments to block(#{block.arity} for 1)"
      end
    end
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;

