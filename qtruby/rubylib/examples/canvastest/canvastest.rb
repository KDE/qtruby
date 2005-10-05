#!/usr/bin/env ruby -w

require 'Qt'
require 'rexml/document'

require '../base/kicons.rb'
require '../base/rui.rb'

class MyCanvasView < Qt::CanvasView
   def initialize(canvas, parent)
      @canvas = canvas
      super(canvas, parent)
   end
   def contentsMousePressEvent(e)
      super
      list = canvas.collisions(e.pos)
      return if list.empty?
      c = list.first
      return if c.rtti != Qt::CanvasItem::Rtti_Rectangle
      c.hide
      @canvas.update
   end
end

class MyWidget < Qt::MainWindow
   slots 'new()', 'open()', 'save_as()'
   def make_rect
      rect = Qt::CanvasRectangle.new(rand(@canvas.width()), rand(@canvas.height()),
                                     @canvas.width / 5, @canvas.width / 5, @canvas)
      z = rand(256)
      color = Qt::Color.new(z,z,z)
      rect.setBrush(Qt::Brush.new(color))
      color = Qt::Color.new(rand(32)*8, rand(32)*8, rand(32)*8)
      rect.setPen(Qt::Pen.new(color, 6))
      rect.setZ(z)
      rect.show
      @rects << rect
   end
   def initialize()
      super

      fileTools = Qt::ToolBar.new(self, "file operations")
      fileMenu = Qt::PopupMenu.new(self)

      actions = [
         RAction.new("&New",  Icons::FILE_NEW, self, SLOT('new()'), [fileTools, fileMenu]),
         RAction.new("&Open...", Icons::FILE_OPEN, self, SLOT('open()'), [fileTools, fileMenu]),
         @save = RAction.new("Save &As...", Icons::FILE_SAVE_AS, self, SLOT('save_as()'), [fileTools, fileMenu]),
         RSeperator.new([fileMenu]),
         RAction.new("E&xit", Icons::EXIT, $qApp, SLOT('quit()'), [fileMenu])
      ]
      build_actions(actions)

      menubar = Qt::MenuBar.new(self)
      menubar.insertItem("&File", fileMenu)

      @canvas = Qt::Canvas.new(640, 480)

      @rects = []
      5.times { make_rect }

      @canvas_view = MyCanvasView.new(@canvas, self)
      self.setCentralWidget(@canvas_view)
      @canvas.update
   end
end

a = Qt::Application.new(ARGV)

w = MyWidget.new
w.show

a.setMainWidget(w)
a.exec()
exit
