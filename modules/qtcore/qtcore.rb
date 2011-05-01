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

require 'qtcore/internal.rb'
require 'qtcore/base.rb'
require 'qtcore/enum.rb'

module Qt

  class Buffer < Qt::Base
    def open(*args)
      method_missing(:open, *args)
    end
  end

  class ByteArray < Qt::Base
    def initialize(*args)
      if args.size == 1 && args[0].kind_of?(String)
        super(args[0], args[0].size)
      else
        super
      end
    end

    def to_s
      return constData()
    end

    def to_i
      return toInt()
    end

    def to_f
      return toDouble()
    end

    def chop(*args)
      method_missing(:chop, *args)
    end

    def split(*args)
      method_missing(:split, *args)
    end
  end

  class ChildEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class CoreApplication < Qt::Base
    def initialize(*args)
      if args.length == 1 && args[0].kind_of?(Array)
        super(args[0].length + 1, [$0] + args[0])
      else
        super(*args)
      end
      $qApp = self
    end

    # Delete the underlying C++ instance after exec returns
    # Otherwise, rb_gc_call_finalizer_at_exit() can delete
    # stuff that Qt::Application still needs for its cleanup.
    def exec
      method_missing(:exec)
      self.dispose
      Qt::Internal.application_terminated = true
    end

    def type(*args)
      method_missing(:type, *args)
    end

    def exit(*args)
      method_missing(:exit, *args)
    end
  end

  class CustomEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Date < Qt::Base
    def initialize(*args)
      if args.size == 1 && args[0].class.name == "Date"
        return super(args[0].year, args[0].month, args[0].day)
      else
        return super(*args)
      end
    end

    def inspect
      str = super
      str.sub(/>$/, " %s>" % toString)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " %s>" % toString)
    end

    def to_date
      ::Date.new! to_julian_day
    end
  end

  class DateTime < Qt::Base
    def initialize(*args)
      if args.size == 1 && args[0].class.name == "DateTime"
        return super(  Qt::Date.new(args[0].year, args[0].month, args[0].day),
                Qt::Time.new(args[0].hour, args[0].min, args[0].sec) )
      elsif args.size == 1 && args[0].class.name == "Time"
        result = super(  Qt::Date.new(args[0].year, args[0].month, args[0].day),
                Qt::Time.new(args[0].hour, args[0].min, args[0].sec, args[0].usec / 1000) )
        result.timeSpec = (args[0].utc? ? Qt::UTC : Qt::LocalTime)
        return result
      else
        return super(*args)
      end
    end

    def to_time
      if timeSpec == Qt::UTC
        return ::Time.utc(  date.year, date.month, date.day,
                  time.hour, time.minute, time.second, time.msec * 1000 )
      else
        return ::Time.local(  date.year, date.month, date.day,
                    time.hour, time.minute, time.second, time.msec * 1000 )
      end
    end

    def inspect
      str = super
      str.sub(/>$/, " %s>" % toString)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " %s>" % toString)
    end
  end

  class Dir < Qt::Base
    Time = Qt::Enum.new(1, "QDir::SortFlag")
  end

  class Event < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class EventLoop < Qt::Base
    def exec(*args)
      method_missing(:exec, *args)
    end

    def exit(*args)
      method_missing(:exit, *args)
    end
  end

  class File < Qt::Base
    def open(*args)
      method_missing(:open, *args)
    end
  end

  class GenericArgument < Qt::Base
    def name(*args)
      method_missing(:name, *args)
    end
  end

  class IODevice < Qt::Base
    def open(*args)
      method_missing(:open, *args)
    end
  end

  class Library < Qt::Base
    def load(*args)
      method_missing(:load, *args)
    end
  end

  class Locale < Qt::Base
    def name(*args)
      method_missing(:name, *args)
    end

    def system(*args)
      method_missing(:system, *args)
    end
  end

  class MetaClassInfo < Qt::Base
    def name(*args)
      method_missing(:name, *args)
    end
  end

  class MetaEnum < Qt::Base
    def name(*args)
      method_missing(:name, *args)
    end

    def keyValues()
      res = []
      for i in 0...keyCount()
        if flag?
          res.push "%s=0x%x" % [key(i), value(i)]
        else
          res.push "%s=%d" % [key(i), value(i)]
        end
      end
      return res
    end

    def inspect
      str = super
      str.sub(/>$/, " scope=%s, name=%s, keyValues=Array (%d element(s))>" % [scope, name, keyValues.length])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " scope=%s, name=%s, keyValues=Array (%d element(s))>" % [scope, name, keyValues.length])
    end
  end

  class MetaMethod < Qt::Base
    # Oops, name clash with the Signal module so hard code
    # this value rather than get it from the Smoke runtime
    Method = Qt::Enum.new(0, "QMetaMethod::MethodType")
    Signal = Qt::Enum.new(1, "QMetaMethod::MethodType")
  end

  class MetaObject < Qt::Base
    def method(*args)
      if args.length == 1 && args[0].kind_of?(Symbol)
        super(*args)
      else
        method_missing(:method, *args)
      end
    end

    # Add three methods, 'propertyNames()', 'slotNames()' and 'signalNames()'
    # from Qt3, as they are very useful when debugging

    def propertyNames(inherits = false)
      res = []
      if inherits
        for p in 0...propertyCount()
          res.push property(p).name
        end
      else
        for p in propertyOffset()...propertyCount()
          res.push property(p).name
        end
      end
      return res
    end

    def slotNames(inherits = false)
      res = []
      if inherits
        for m in 0...methodCount()
          if method(m).methodType == Qt::MetaMethod::Slot
            res.push "%s %s" % [method(m).typeName == "" ? "void" : method(m).typeName,
                      method(m).signature]
          end
        end
      else
        for m in methodOffset()...methodCount()
          if method(m).methodType == Qt::MetaMethod::Slot
            res.push "%s %s" % [method(m).typeName == "" ? "void" : method(m).typeName,
                      method(m).signature]
          end
        end
      end
      return res
    end

    def signalNames(inherits = false)
      res = []
      if inherits
        for m in 0...methodCount()
          if method(m).methodType == Qt::MetaMethod::Signal
            res.push "%s %s" % [method(m).typeName == "" ? "void" : method(m).typeName,
                      method(m).signature]
          end
        end
      else
        for m in methodOffset()...methodCount()
          if method(m).methodType == Qt::MetaMethod::Signal
            res.push "%s %s" % [method(m).typeName == "" ? "void" : method(m).typeName,
                      method(m).signature]
          end
        end
      end
      return res
    end

    def enumerators(inherits = false)
      res = []
      if inherits
        for e in 0...enumeratorCount()
          res.push enumerator(e)
        end
      else
        for e in enumeratorOffset()...enumeratorCount()
          res.push enumerator(e)
        end
      end
      return res
    end

    def inspect
      str = super
      str.sub!(/>$/, "")
      str << " className=%s," % className
      str << " propertyNames=Array (%d element(s))," % propertyNames.length unless propertyNames.length == 0
      str << " signalNames=Array (%d element(s))," % signalNames.length unless signalNames.length == 0
      str << " slotNames=Array (%d element(s))," % slotNames.length unless slotNames.length == 0
      str << " enumerators=Array (%d element(s))," % enumerators.length unless enumerators.length == 0
      str << " superClass=%s," % superClass.inspect unless superClass == nil
      str.chop!
      str << ">"
    end

    def pretty_print(pp)
      str = to_s
      str.sub!(/>$/, "")
      str << "\n className=%s," % className
      str << "\n propertyNames=Array (%d element(s))," % propertyNames.length unless propertyNames.length == 0
      str << "\n signalNames=Array (%d element(s))," % signalNames.length unless signalNames.length == 0
      str << "\n slotNames=Array (%d element(s))," % slotNames.length unless slotNames.length == 0
      str << "\n enumerators=Array (%d element(s))," % enumerators.length unless enumerators.length == 0
      str << "\n superClass=%s," % superClass.inspect unless superClass == nil
      str << "\n methodCount=%d," % methodCount
      str << "\n methodOffset=%d," % methodOffset
      str << "\n propertyCount=%d," % propertyCount
      str << "\n propertyOffset=%d," % propertyOffset
      str << "\n enumeratorCount=%d," % enumeratorCount
      str << "\n enumeratorOffset=%d," % enumeratorOffset
      str.chop!
      str << ">"
      pp.text str
    end
  end

  class MetaProperty < Qt::Base
    def name(*args)
      method_missing(:name, *args)
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class MetaType < Qt::Base
    Float = Qt::Enum.new(135, "QMetaType::Type")

    def load(*args)
      method_missing(:load, *args)
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Object < Qt::Base
  end

  class PluginLoader < Qt::Base
    def load(*args)
      method_missing(:load, *args)
    end
  end

  class Point < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " x=%d, y=%d>" % [self.x, self.y])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n x=%d,\n y=%d>" % [self.x, self.y])
    end
  end

  class PointF < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " x=%f, y=%f>" % [self.x, self.y])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n x=%f,\n y=%f>" % [self.x, self.y])
    end
  end

  class Process < Qt::Base
    StandardError = Qt::Enum.new(1, "QProcess::ProcessChannel")
  end

  class Line < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " x1=%d, y1=%d, x2=%d, y2=%d>" % [x1, y1, x2, y2])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n x1=%d,\n y1=%d,\n x2=%d,\n y2=%d>" % [x1, y1, x2, y2])
    end
  end

  class LineF < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " x1=%f, y1=%f, x2=%f, y2=%f>" % [x1, y1, x2, y2])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n x1=%f,\n y1=%f,\n x2=%f,\n y2=%f>" % [x1, y1, x2, y2])
    end
  end

  class MetaType < Qt::Base
    def self.type(*args)
      method_missing(:type, *args)
    end
  end

  class ModelIndex < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " valid?=%s, row=%s, column=%s>" % [valid?, row, column])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n valid?=%s,\n row=%s,\n column=%s>" % [valid?, row, column])
    end
  end

  class Rect < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " x=%d, y=%d, width=%d, height=%d>" % [self.x, self.y, width, height])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n x=%d,\n y=%d,\n width=%d,\n height=%d>" % [self.x, self.y, width, height])
    end
  end

  class RectF < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " x=%f, y=%f, width=%f, height=%f>" % [self.x, self.y, width, height])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n x=%f,\n y=%f,\n width=%f,\n height=%f>" % [self.x, self.y, width, height])
    end
  end

  class Size < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " width=%d, height=%d>" % [width, height])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n width=%d,\n height=%d>" % [width, height])
    end
  end

  class SizeF < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " width=%f, height=%f>" % [width, height])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n width=%f,\n height=%f>" % [width, height])
    end
  end

  class TemporaryFile < Qt::Base
    def open(*args)
      method_missing(:open, *args)
    end
  end

  class Time < Qt::Base
    def initialize(*args)
      if args.size == 1 && args[0].class.name == "Time"
        return super(args[0].hour, args[0].min, args[0].sec)
      else
        return super(*args)
      end
    end

    def inspect
      str = super
      str.sub(/>$/, " %s>" % toString)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " %s>" % toString)
    end
  end

  class TimerEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class TimeLine < Qt::Base
    def frameRange=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class Translator < Qt::Base
    def load(*args)
      method_missing(:load, *args)
    end
  end

  class Uuid < Qt::Base
    Time = Qt::Enum.new(1, "QUuid::Version")
  end

  class Variant < Qt::Base
    String = Qt::Enum.new(10, "QVariant::Type")
    Date = Qt::Enum.new(14, "QVariant::Type")
    Time = Qt::Enum.new(15, "QVariant::Type")
    DateTime = Qt::Enum.new(16, "QVariant::Type")

    def initialize(*args)
      if args.size == 1 && args[0].nil?
        return super()
      elsif args.size == 1 && args[0].class.name == "Date"
        return super(Qt::Date.new(args[0]))
      elsif args.size == 1 && args[0].class.name == "DateTime"
        return super(Qt::DateTime.new(  Qt::Date.new(args[0].year, args[0].month, args[0].day),
                        Qt::Time.new(args[0].hour, args[0].min, args[0].sec) ) )
      elsif args.size == 1 && args[0].class.name == "Time"
        return super(Qt::Time.new(args[0]))
      elsif args.size == 1 && args[0].class.name == "BigDecimal"
        return super(args[0].to_f) # we have to make do with a float
      else
        return super(*args)
      end
    end

    def to_a
      return toStringList()
    end

    def to_f
      return toDouble()
    end

    def to_i
      return toInt()
    end

    def to_int
      return toInt()
    end

    def value
      case type()
      when Qt::Variant::Invalid
        return nil
      when Qt::Variant::Bitmap
      when Qt::Variant::Bool
        return toBool
      when Qt::Variant::Brush
        return qVariantValue(Qt::Brush, self)
      when Qt::Variant::ByteArray
        return toByteArray
      when Qt::Variant::Char
        return qVariantValue(Qt::Char, self)
      when Qt::Variant::Color
        return qVariantValue(Qt::Color, self)
      when Qt::Variant::Cursor
        return qVariantValue(Qt::Cursor, self)
      when Qt::Variant::Date
        return toDate
      when Qt::Variant::DateTime
        return toDateTime
      when Qt::Variant::Double
        return toDouble
      when Qt::Variant::Font
        return qVariantValue(Qt::Font, self)
      when Qt::Variant::Icon
        return qVariantValue(Qt::Icon, self)
      when Qt::Variant::Image
        return qVariantValue(Qt::Image, self)
      when Qt::Variant::Int
        return toInt
      when Qt::Variant::KeySequence
        return qVariantValue(Qt::KeySequence, self)
      when Qt::Variant::Line
        return toLine
      when Qt::Variant::LineF
        return toLineF
      when Qt::Variant::List
        return toList
      when Qt::Variant::Locale
        return qVariantValue(Qt::Locale, self)
      when Qt::Variant::LongLong
        return toLongLong
      when Qt::Variant::Map
        return toMap
      when Qt::Variant::Palette
        return qVariantValue(Qt::Palette, self)
      when Qt::Variant::Pen
        return qVariantValue(Qt::Pen, self)
      when Qt::Variant::Pixmap
        return qVariantValue(Qt::Pixmap, self)
      when Qt::Variant::Point
        return toPoint
      when Qt::Variant::PointF
        return toPointF
      when Qt::Variant::Polygon
        return qVariantValue(Qt::Polygon, self)
      when Qt::Variant::Rect
        return toRect
      when Qt::Variant::RectF
        return toRectF
      when Qt::Variant::RegExp
        return toRegExp
      when Qt::Variant::Region
        return qVariantValue(Qt::Region, self)
      when Qt::Variant::Size
        return toSize
      when Qt::Variant::SizeF
        return toSizeF
      when Qt::Variant::SizePolicy
        return toSizePolicy
      when Qt::Variant::String
        return toString
      when Qt::Variant::StringList
        return toStringList
      when Qt::Variant::TextFormat
        return qVariantValue(Qt::TextFormat, self)
      when Qt::Variant::TextLength
        return qVariantValue(Qt::TextLength, self)
      when Qt::Variant::Time
        return toTime
      when Qt::Variant::UInt
        return toUInt
      when Qt::Variant::ULongLong
        return toULongLong
      when Qt::Variant::Url
        return toUrl
      end

      return qVariantValue(nil, self)
    end

    def inspect
      str = super
      str.sub(/>$/, " typeName=%s>" % typeName)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " typeName=%s>" % typeName)
    end

    def load(*args)
      method_missing(:load, *args)
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  # These values are from the enum WindowType in qnamespace.h.
  # Some of the names such as 'Qt::Dialog', clash with QtRuby
  # class names. So add some constants here to use instead,
  # renamed with an ending of 'Type'.
  WidgetType = 0x00000000
  WindowType = 0x00000001
  DialogType = 0x00000002 | WindowType
  SheetType = 0x00000004 | WindowType
  DrawerType = 0x00000006 | WindowType
  PopupType = 0x00000008 | WindowType
  ToolType = 0x0000000a | WindowType
  ToolTipType = 0x0000000c | WindowType
  SplashScreenType = 0x0000000e | WindowType
  DesktopType = 0x00000010 | WindowType
  SubWindowType =  0x00000012

end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;

