=begin
     Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>

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

  # If a C++ enum was converted to an ordinary ruby Integer, the
  # name of the type is lost. The enum type name is needed for overloaded
  # method resolution when two methods differ only by an enum type.
  class Enum
    attr_accessor :type, :value
    def initialize(n, enum_type)
      @value = n
      @type = enum_type
    end

    def +(n)
      return @value + n.to_i
    end
    def -(n)
      return @value - n.to_i
    end
    def *(n)
      return @value * n.to_i
    end
    def /(n)
      return @value / n.to_i
    end
    def %(n)
      return @value % n.to_i
    end
    def **(n)
      return @value ** n.to_i
    end

    def |(n)
      return Enum.new(@value | n.to_i, @type)
    end
    def &(n)
      return Enum.new(@value & n.to_i, @type)
    end
    def ^(n)
      return Enum.new(@value ^ n.to_i, @type)
    end
    def ~()
      return ~ @value
    end
    def <(n)
      return @value < n.to_i
    end
    def <=(n)
      return @value <= n.to_i
    end
    def >(n)
      return @value > n.to_i
    end
    def >=(n)
      return @value >= n.to_i
    end
    def <<(n)
      return Enum.new(@value << n.to_i, @type)
    end
    def >>(n)
      return Enum.new(@value >> n.to_i, @type)
    end

    def ==(n) return @value == n.to_i end
    def to_i() return @value end

    def to_f() return @value.to_f end
    def to_s() return @value.to_s end

    def coerce(n)
      [n, @value]
    end

    def inspect
      to_s
    end

    def pretty_print(pp)
      pp.text "#<%s:0x%8.8x @type=%s, @value=%d>" % [self.class.name, object_id, type, value]
    end
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;