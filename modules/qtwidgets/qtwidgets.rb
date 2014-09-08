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

    class AbstractSlider < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class AbstractTextDocumentLayout < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class AccessibleEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class ActionEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Application < Qt::Base
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
  end

  class ButtonGroup < Qt::Base
    def id(*args)
      method_missing(:id, *args)
    end
  end

  class CloseEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Color < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " %s>" % name)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " %s>" % name)
    end

    def name(*args)
      method_missing(:name, *args)
    end
  end

  class ContextMenuEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Cursor < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " shape=%d>" % shape)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " shape=%d>" % shape)
    end
  end

  class Dial < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class Dialog < Qt::Base
    def exec(*args)
      method_missing(:exec, *args)
    end
  end

  class DoubleSpinBox < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class DoubleValidator < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class DragEnterEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class DragLeaveEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class DropEvent < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class FileIconProvider < Qt::Base
    File = Qt::Enum.new(6, "QFileIconProvider::IconType")

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class FileOpenEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class FocusEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Font < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " family=%s, pointSize=%d, weight=%d, italic=%s, bold=%s, underline=%s, strikeOut=%s>" %
      [family.inspect, pointSize, weight, italic, bold, underline, strikeOut])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n family=%s,\n pointSize=%d,\n weight=%d,\n italic=%s,\n bold=%s,\n underline=%s,\n strikeOut=%s>" %
      [family.inspect, pointSize, weight, italic, bold, underline, strikeOut])
    end
  end

  class FontDatabase < Qt::Base
    Symbol = Qt::Enum.new(30, "QFontDatabase::WritingSystem")
  end

  class Gradient < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsEllipseItem < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsItem < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsItemGroup < Qt::Base
    Type = 10

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsLineItem < Qt::Base
    Type = 6
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsPathItem < Qt::Base
    Type = 2
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsPixmapItem < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsPolygonItem < Qt::Base
    Type = 5
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsProxyWidget < Qt::Base
    Type = 12
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsRectItem < Qt::Base
    Type = 3
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSceneMouseEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSceneContextMenuEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSceneHoverEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSceneHelpEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSceneWheelEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSimpleTextItem < Qt::Base
    Type = 9
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSvgItem < Qt::Base
    Type = 13
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsTextItem < Qt::Base
    Type = 8
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsWidget < Qt::Base
    Type = 11
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class HelpEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class HideEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class HoverEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class IconDragEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class InputEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class InputMethodEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Image < Qt::Base
    def fromImage(image)
      send("operator=".to_sym, image)
    end

    def format(*args)
      method_missing(:format, *args)
    end

    def load(*args)
      method_missing(:load, *args)
    end
  end

  class ImageIOHandler < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end

    def name(*args)
      method_missing(:name, *args)
    end
  end

  class ImageReader < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class ImageWriter < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class IntValidator < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class ItemSelection < Qt::Base
    include Enumerable

    def each
      for i in 0...count
        yield at(i)
      end
      return self
    end

    def select(*args)
      method_missing(:select, *args)
    end

    def split(*args)
      method_missing(:split, *args)
    end
  end

  class ItemSelectionModel < Qt::Base
    def select(*args)
      method_missing(:select, *args)
    end
  end

  class KeyEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class KeySequence < Qt::Base
    def initialize(*args)
      if args.length == 1 && args[0].kind_of?(Qt::Enum) && args[0].type == "Qt::Key"
        return super(args[0].to_i)
      end
      return super(*args)
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

  class LCDNumber < Qt::Base
    def display(item)
      method_missing(:display, item)
    end
  end

  class Library < Qt::Base
    def load(*args)
      method_missing(:load, *args)
    end
  end

  class ListWidgetItem < Qt::Base
    def clone(*args)
      Qt::ListWidgetItem.new(self)
    end

    def type(*args)
      method_missing(:type, *args)
    end

    def inspect
      str = super
      str.sub(/>$/, " text='%s'>" % text)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " text='%s'>" % text)
    end
  end

  class Menu < Qt::Base
    def exec(*args)
      method_missing(:exec, *args)
    end
  end

  class MouseEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class MoveEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Movie < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class PageSetupDialog < Qt::Base
    def exec(*args)
      method_missing(:exec, *args)
    end
  end

  class PaintEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Picture < Qt::Base
    def load(*args)
      method_missing(:load, *args)
    end
  end

  class PictureIO < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class Pixmap < Qt::Base
    def load(*args)
      method_missing(:load, *args)
    end
  end

  class Polygon < Qt::Base
    include Enumerable

    def each
      for i in 0...count
        yield point(i)
      end
      return self
    end
  end

  class PolygonF < Qt::Base
    include Enumerable

    def each
      for i in 0...count
        yield point(i)
      end
      return self
    end
  end

  class PrintDialog < Qt::Base
    def exec(*args)
      method_missing(:exec, *args)
    end
  end

  class ProgressBar < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class ProgressDialog < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class Printer < Qt::Base
    def abort(*args)
      method_missing(:abort, *args)
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

  class ResizeEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class ScrollBar < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class Shortcut < Qt::Base
    def id(*args)
      method_missing(:id, *args)
    end
  end

  class ShortcutEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class ShowEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class SizePolicy < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " horizontalPolicy=%d, verticalPolicy=%d>" % [horizontalPolicy, verticalPolicy])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n horizontalPolicy=%d,\n verticalPolicy=%d>" % [horizontalPolicy, verticalPolicy])
    end
  end

  class Slider < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class SpinBox < Qt::Base
    def range=(arg)
      if arg.kind_of? Range
        return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
      else
        return super(arg)
      end
    end
  end

  class StandardItem < Qt::Base
    def inspect
      str = super
      str.sub(/>$/, " text='%s'>" % [text])
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, "\n text='%s'>" % [text])
    end

    def type(*args)
      method_missing(:type, *args)
    end

    def clone
      Qt::StandardItem.new(self)
    end
  end

  class StandardItemModel < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class StatusTipEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class StyleHintReturn < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class StyleOption < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class SyntaxHighlighter < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class TableWidgetItem < Qt::Base
    def clone(*args)
      Qt::TableWidgetItem.new(self)
    end

    def type(*args)
      method_missing(:type, *args)
    end

    def inspect
      str = super
      str.sub(/>$/, " text='%s'>" % text)
    end

    def pretty_print(pp)
      str = to_s
      pp.text str.sub(/>$/, " text='%s'>" % text)
    end
  end

  class TextCursor < Qt::Base
    def select(*k)
      method_missing(:select, *k)
    end
  end

  class TextDocument < Qt::Base
    def clone(*args)
      method_missing(:clone, *args)
    end

    def print(*args)
      method_missing(:print, *args)
    end
  end

  class TextFormat < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class TextImageFormat < Qt::Base
    def name(*args)
      method_missing(:name, *args)
    end
  end

  class TextInlineObject < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class TextLength < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class TextList < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class TextObject < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class TextTable < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class TextTableCell < Qt::Base
    def format(*args)
      method_missing(:format, *args)
    end
  end

  class TreeWidget < Qt::Base
    include Enumerable

    def each
      it = Qt::TreeWidgetItemIterator.new(self)
      while it.current
        yield it.current
        it += 1
      end
    end
  end

  class TreeWidgetItem < Qt::Base
    include Enumerable

    def initialize(*args)
      # There is not way to distinguish between the copy constructor
      # QTreeWidgetItem (const QTreeWidgetItem & other)
      # and
      # QTreeWidgetItem (QTreeWidgetItem * parent, const QStringList & strings, int type = Type)
      # when the latter has a single argument. So force the second variant to be called
      if args.length == 1 && args[0].kind_of?(Qt::TreeWidgetItem)
        super(args[0], Qt::TreeWidgetItem::Type)
      else
        super(*args)
      end
    end

    def inspect
      str = super
      str.sub!(/>$/, "")
      str << " parent=%s," % parent unless parent.nil?
      for i in 0..(columnCount - 1)
        str << " text%d='%s'," % [i, self.text(i)]
      end
      str.sub!(/,?$/, ">")
    end

    def pretty_print(pp)
      str = to_s
      str.sub!(/>$/, "")
      str << " parent=%s," % parent unless parent.nil?
      for i in 0..(columnCount - 1)
        str << " text%d='%s'," % [i, self.text(i)]
      end
      str.sub!(/,?$/, ">")
      pp.text str
    end

    def clone(*args)
      Qt::TreeWidgetItem.new(self)
    end

    def type(*args)
      method_missing(:type, *args)
    end

    def each
      it = Qt::TreeWidgetItemIterator.new(self)
      while it.current
        yield it.current
        it += 1
      end
    end
  end

  class TreeWidgetItemIterator < Qt::Base
    def current
      return send("operator*".to_sym)
    end
  end

  class WhatsThisClickedEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Widget < Qt::Base
    def raise(*args)
      method_missing(:raise, *args)
    end
  end

  class WindowStateChangeEvent < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end
end
