=begin
/*
 *   Copyright 2003-20011 by Richard Dale <richard.j.dale@gmail.com>

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
=end

module Qt
  class DBusArgument < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " currentSignature='%s', atEnd=%s>" % [currentSignature, atEnd])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " currentSignature='%s, atEnd=%s'>" % [currentSignature, atEnd])
    end
  end

  class DBusConnection < Qt::Base
    def send(*args)
      method_missing(:send, *args)
    end
  end

  class DBusConnectionInterface < Qt::Base
    def serviceOwner(name)
        return Qt::DBusReply.new(internalConstCall(Qt::DBus::AutoDetect, "GetNameOwner", [Qt::Variant.new(name)]))
    end

    def registeredServiceNames
      return Qt::DBusReply.new(internalConstCall(Qt::DBus::AutoDetect, "ListNames"))
    end

    def isServiceRegistered(serviceName)
        return Qt::DBusReply.new(internalConstCall(Qt::DBus::AutoDetect, "NameHasOwner", [Qt::Variant.new(serviceName)]))
    end

    def serviceRegistered?(serviceName)
        return isServiceRegistered(serviceName)
    end

    def servicePid(serviceName)
        return Qt::DBusReply.new(internalConstCall(Qt::DBus::AutoDetect, "GetConnectionUnixProcessID", [Qt::Variant.new(serviceName)]))
    end

    def serviceUid(serviceName)
        return Qt::DBusReply.new(internalConstCall(Qt::DBus::AutoDetect, "GetConnectionUnixUser", [Qt::Variant.new(serviceName)]))
    end

    def startService(name)
        return call("StartServiceByName", Qt::Variant.new(name), Qt::Variant.new(0)).value
    end
  end

  class DBusError < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class DBusInterface < Qt::Base
    def call(method_name, *args)
      if args.length == 0
        return super(method_name)
      elsif method_name.is_a? Qt::Enum
        opt = args.shift
        qdbusArgs = args.collect {|arg| qVariantFromValue(arg)}
        return super(method_name, opt, *qdbusArgs)
      else
        # If the method is Qt::DBusInterface.call(), create an Array
        # 'dbusArgs' of Qt::Variants from '*args'
        qdbusArgs = args.collect {|arg| qVariantFromValue(arg)}
        return super(method_name, *qdbusArgs)
      end
    end

    def method_missing(id, *args)
      begin
        # First look for a method in the Smoke runtime
        # If not found, then throw an exception and try dbus.
        super(id, *args)
      rescue
        if args.length == 0
          return call(id.to_s).value
        else
          return call(id.to_s, *args).value
        end
      end
    end
  end

  class DBusMessage < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end

    def value
      if type() == Qt::DBusMessage::ReplyMessage
        reply = arguments()
        if reply.length == 0
          return nil
        elsif reply.length == 1
          return reply[0].value
        else
          return reply.collect {|v| v.value}
        end
      else
        return nil
      end
    end

    def <<(a)
      if a.kind_of?(Qt::Variant)
        return super(a)
      else
        return super(qVariantFromValue(a))
      end
    end
  end

  class DBusReply
    def initialize(reply)
      @error = Qt::DBusError.new(reply)

      if @error.valid?
        @data = Qt::Variant.new
        return
      end

      if reply.arguments.length >= 1
        @data = reply.arguments[0]
        return
      end

      # error
      @error = Qt::DBusError.new(  Qt::DBusError::InvalidSignature,
                    "Unexpected reply signature" )
      @data = Qt::Variant.new      # clear it
    end

    def isValid
      return !@error.isValid
    end

    def valid?
      return !@error.isValid
    end

    def value
      return @data.value
    end

    def error
      return @error
    end
  end

  class DBusVariant < Variant
    def initialize(value)
      if value.kind_of? Qt::Variant
        super(value)
      else
        super(Qt::Variant.new(value))
      end
    end

    def setVariant(variant)
    end

    def variant=(variant)
      setVariant(variant)
    end

    def variant()
      return self
    end
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;