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

module PlasmaScriptengineRuby
  class Applet < Plasma::AppletScript
    def initialize(parent, args)
      super(parent)
    end

    def camelize(str)
      str.gsub(/(^|[_-])(.)/) { $2.upcase }
    end

    def init
      applet.resize(200, 200)

      puts "RubyAppletScript::Applet#init mainScript: #{mainScript}"
      program = Qt::FileInfo.new(mainScript)
      $: << program.path
      load Qt::File.encodeName(program.filePath).to_s
      moduleName = camelize(Qt::Dir.new(package.path).dirName)
      className = camelize(program.baseName)
      puts "RubyAppletScript::Applet#init instantiating: #{moduleName}::#{className}"
      klass = Object.const_get(moduleName.to_sym).const_get(className.to_sym)
      @applet_script = klass.new(self)
      @applet_script.init

      set_up_event_handlers
      return true
    end

    def paintInterface(painter, option, contentsRect)
      @applet_script.paintInterface(painter, option, contentsRect)
    end

    def constraintsEvent(constraints)
      @applet_script.constraintsEvent(constraints)
    end

    def contextualActions
      @applet_script.contextualActions
    end

    def showConfigurationInterface
      @applet_script.showConfigurationInterface
    end

    protected

    def eventFilter(obj, event)
      handler = @event_handlers[event.type.to_i]
      if handler
        @applet_script.send(handler, event)
        return true
      else
        return false
      end
    end

    private

    def set_up_event_handlers
      @event_handlers = {}

      if @applet_script.respond_to?(:mousePressEvent)
        @event_handlers[Qt::Event::GraphicsSceneMousePress.to_i] = :mousePressEvent
      end

      if @applet_script.respond_to?(:contextMenuEvent)
        @event_handlers[Qt::Event::GraphicsSceneContextMenu.to_i] = :contextMenuEvent
      end

      if @applet_script.respond_to?(:dragEnterEvent)
        @event_handlers[Qt::Event::GraphicsSceneDragEnter.to_i] = :dragEnterEvent
      end

      if @applet_script.respond_to?(:dragLeaveEvent)
        @event_handlers[Qt::Event::GraphicsSceneDragLeave.to_i] = :dragLeaveEvent
      end

      if @applet_script.respond_to?(:dragMoveEvent)
        @event_handlers[Qt::Event::GraphicsSceneDragMove.to_i] = :dragMoveEvent
      end

      if @applet_script.respond_to?(:dropEvent)
        @event_handlers[Qt::Event::GraphicsSceneDrop.to_i] = :dropEvent
      end

      if @applet_script.respond_to?(:focusInEvent)
        @event_handlers[Qt::Event::FocusIn.to_i] = :focusInEvent
      end

      if @applet_script.respond_to?(:focusOutEvent)
        @event_handlers[Qt::Event::FocusOut.to_i] = :focusOutEvent
      end

      if @applet_script.respond_to?(:hoverEnterEvent)
        @event_handlers[Qt::Event::GraphicsSceneHoverEnter.to_i] = :hoverEnterEvent
      end

      if @applet_script.respond_to?(:hoverLeaveEvent)
        @event_handlers[Qt::Event::GraphicsSceneHoverLeave.to_i] = :hoverLeaveEvent
      end

      if @applet_script.respond_to?(:hoverMoveEvent)
        @event_handlers[Qt::Event::GraphicsSceneHoverMove.to_i] = :hoverMoveEvent
      end

      if @applet_script.respond_to?(:inputMethodEvent)
        @event_handlers[Qt::Event::InputMethod.to_i] = :inputMethodEvent
      end

      if @applet_script.respond_to?(:keyPressEvent)
        @event_handlers[Qt::Event::KeyPress.to_i] = :keyPressEvent
      end

      if @applet_script.respond_to?(:keyReleaseEvent)
        @event_handlers[Qt::Event::KeyRelease.to_i] = :keyReleaseEvent
      end

      if @applet_script.respond_to?(:mouseDoubleClickEvent)
        @event_handlers[Qt::Event::GraphicsSceneMouseDoubleClick.to_i] = :mouseDoubleClickEvent
      end

      if @applet_script.respond_to?(:mouseMoveEvent)
        @event_handlers[Qt::Event::GraphicsSceneMouseMove.to_i] = :mouseMoveEvent
      end

      if @applet_script.respond_to?(:mousePressEvent)
        @event_handlers[Qt::Event::GraphicsSceneMousePress.to_i] = :mousePressEvent
      end

      if @applet_script.respond_to?(:mouseReleaseEvent)
        @event_handlers[Qt::Event::GraphicsSceneMouseRelease.to_i] = :mouseReleaseEvent
      end

      if @applet_script.respond_to?(:wheelEvent)
        @event_handlers[Qt::Event::GraphicsSceneWheel.to_i] = :wheelEvent
      end

      if !@event_handlers.empty?
        applet.installEventFilter(self)
      end
    end

  end
end

module Plasma
  #
  # Because a PlasmaScript::Applet is not actually a Plasma::Applet we
  # need to 'cheat' in the api, to pretend that it is. So the constructors
  # in the Plasma widget classes will substitute any PlasmaScript::Applet
  # argument passed for the real Plasma::Applet in the ScriptEngine
  #

  class CheckBox < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class ComboBox < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Flash < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Frame < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class GroupBox < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Icon < Qt::Base
    def initialize(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end
  end

  class Label < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class LineEdit < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Meter < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class PushButton < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class RadioButton < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class SignalPlotter < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Slider < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class TabBar < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class TextEdit < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class WebContent < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

end

module Qt
  class GraphicsWidget < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end
  end

  class GraphicsGridLayout < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end

    def addItem(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end

    def alignment(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def setAlignment(item, alignment)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet, alignment)
      else
        super
      end
    end
  end

  class GraphicsLinearLayout < Qt::Base
    def initialize(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end

    def addItem(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end

    def alignment(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def insertItem(index, item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(index, item.applet_script.applet)
      else
        super
      end
    end

    def setAlignment(item, alignment)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet, alignment)
      else
        super
      end
    end

    def setStretchFactor(item, stretch)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet, stretch)
      else
        super
      end
    end

    def stretchFactor(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end
  end
end