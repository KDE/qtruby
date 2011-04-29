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
  class Base
    def **(a)
      return Qt::**(self, a)
    end
    def +(a)
      return Qt::+(self, a)
    end
    def ~(a)
      return Qt::~(self, a)
    end
    def -@()
      return Qt::-(self)
    end
    def -(a)
      return Qt::-(self, a)
    end
    def *(a)
      return Qt::*(self, a)
    end
    def /(a)
      return Qt::/(self, a)
    end
    def %(a)
      return Qt::%(self, a)
    end
    def >>(a)
      return Qt::>>(self, a)
    end
    def <<(a)
      return Qt::<<(self, a)
    end
    def &(a)
      return Qt::&(self, a)
    end
    def ^(a)
      return Qt::^(self, a)
    end
    def |(a)
      return Qt::|(self, a)
    end

    # Module has '<', '<=', '>' and '>=' operator instance methods, so pretend they
    # don't exist by calling method_missing() explicitely
    def <(a)
      begin
        Qt::method_missing(:<, self, a)
      rescue
        super(a)
      end
    end

    def <=(a)
      begin
        Qt::method_missing(:<=, self, a)
      rescue
        super(a)
      end
    end

    def >(a)
      begin
        Qt::method_missing(:>, self, a)
      rescue
        super(a)
      end
    end

    def >=(a)
      begin
          Qt::method_missing(:>=, self, a)
      rescue
          super(a)
      end
    end

    # Object has a '==' operator instance method, so pretend it
    # don't exist by calling method_missing() explicitely
    def ==(a)
      return false if a.nil?
      begin
        Qt::method_missing(:==, self, a)
      rescue
        super(a)
      end
    end

=begin
    def self.ancestors
      klass = self
      classid = nil
      loop do
        classid = Qt::Internal::find_pclassid(klass.name)
        break if classid.index

        klass = klass.superclass
        if klass.nil?
            return super
        end
      end

      klasses = super
      klasses.delete(Qt::Base)
      klasses.delete(self)
      ids = []
      Qt::Internal::getAllParents(classid, ids)
      return [self] + ids.map {|id| Qt::Internal.find_class(Qt::Internal.classid2name(id))} + klasses
    end

    # Change the behaviors of is_a? and kind_of? (alias of is_a?) to use above self.ancestors method
    # Note: this definition also affects Object#===
    def is_a?(mod)
      super || self.class.ancestors.include?(mod)
    end
    alias :kind_of? :is_a?

    def methods(regular=true)
      if !regular
        return singleton_methods
      end

      qt_methods(super, 0x0)
    end

    def protected_methods(all=true)
      # From smoke.h, Smoke::mf_protected 0x80
      qt_methods(super, 0x80)
    end

    def public_methods(all=true)
      methods
    end

    def singleton_methods(all=true)
      # From smoke.h, Smoke::mf_static 0x01
      qt_methods(super, 0x01)
    end

    private
    def qt_methods(meths, flags)
      ids = []
      # These methods are all defined in Qt::Base, even if they aren't supported by a particular
      # subclass, so remove them to avoid confusion
      meths -= ["%", "&", "*", "**", "+", "-", "-@", "/", "<", "<<", "<=", ">", ">=", ">>", "|", "~", "^"]
      classid = Qt::Internal::idInstance(self)
      Qt::Internal::getAllParents(classid, ids)
      ids << classid
      ids.each { |c| Qt::Internal::findAllMethodNames(meths, c, flags) }
      return meths.uniq
    end
=end
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
