=begin
/***************************************************************************
                          plasma.rb  -  Plasma applet helper code
                             -------------------
    begin                : Sat April 5 2008
    copyright            : (C) 2008 by Richard Dale
    email                : richard.j.dale@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
=end

require 'qtwebkit'

module Plasma

  module Internal
    def self.init_all_classes
      Qt::Internal::add_normalize_proc(Proc.new do |classname|
        if classname =~ /^Plasma/
          now = classname
        end
        now
      end)
      getClassList.each do |c|
        classname = Qt::Internal::normalize_classname(c)
        id = Qt::Internal::findClass(c);
        Qt::Internal::insert_pclassid(classname, id)
        Qt::Internal::cpp_names[classname] = c
        klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, Plasma) \
                                           : Qt::Internal::create_qt_class(classname, Plasma)
        Qt::Internal::classes[classname] = klass unless klass.nil?
      end
    end
  end

  class Applet < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class Containment < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class PackageMetadata < Qt::Base
    def type(*args)
      method_missing(:type, *args)
    end
  end

  class PackageStructure < Qt::Base
    def type(*args)
      method_missing(:type, *args)
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

end

module PlasmaScripting
  class Applet < Qt::Object
  slots  "setImmutability(Plasma::ImmutabilityType)",
            :destroy,
            :showConfigurationInterface,
            :raise,
            :lower,
            :flushPendingConstraintsEvents,
            :init

    signals :releaseVisualFocus,
            :geometryChanged,
            :configNeedsSaving,
            :activate

    attr_accessor :applet_script

    def initialize(parent, args = nil)
      super(parent)
      @applet_script = parent
      connect(@applet_script.applet, SIGNAL(:releaseVisualFocus), self, SIGNAL(:releaseVisualFocus))
      connect(@applet_script.applet, SIGNAL(:geometryChanged), self, SIGNAL(:geometryChanged))
      connect(@applet_script.applet, SIGNAL(:configNeedsSaving), self, SIGNAL(:configNeedsSaving))
      connect(@applet_script.applet, SIGNAL(:activate), self, SIGNAL(:activate))
    end

    # If a method is called on a PlasmaScripting::Applet instance is found to be missing
    # then try calling the method on the underlying Plasma::Applet in the ScriptEngine.
    def method_missing(method, *args)
      begin
        super(method, *args)
      rescue
        applet_script.applet.method_missing(method, *args)
      end
    end

    def paintInterface(painter, option, contentsRect)
    end

    def size
      @applet_script.size
    end

    def shape
      @applet_script.shape
    end

    def constraintsEvent(constraints)
    end

    def contextualActions
      return []
    end

    def createConfigurationInterface(dialog)
    end

    def showConfigurationInterface
        dialogId = "#{applet.id}settings#{applet.name}"
        windowTitle = KDE::i18nc("@title:window", "%s Settings" % applet.name)
        @nullManager = KDE::ConfigSkeleton.new(nil)
        dialog = KDE::ConfigDialog.new(nil, dialogId, @nullManager)
        dialog.faceType = KDE::PageDialog::Auto
        dialog.windowTitle = windowTitle
        dialog.setAttribute(Qt::WA_DeleteOnClose, true)
        createConfigurationInterface(dialog)
        # TODO: would be nice to not show dialog if there are no pages added?
        connect(dialog, SIGNAL(:finished), @nullManager, SLOT(:deleteLater))
        # TODO: Apply button does not correctly work for now, so do not show it
        dialog.showButton(KDE::Dialog::Apply, false)
        dialog.show
    end

    def dataEngine(engine)
      @applet_script.dataEngine(engine)
    end

    def package
      @applet_script.package
    end

  def setImmutability(immutabilityType)
      @applet_script.applet.setImmutability(immutabilityType)
    end

  def immutability=(immutabilityType)
      setImmutability(immutabilityType)
    end

    def destroy
      @applet_script.applet.destroy
    end

    def raise
      @applet_script.applet.raise
    end

    def lower
      @applet_script.applet.lower
    end

    def flushPendingConstraintsEvents
      @applet_script.applet.flushPendingConstraintsEvents
    end
  end

  class DataEngine < Qt::Object
    signals "sourceAdded(QString)", "sourceRemoved(QString)"

    attr_accessor :data_engine_script

    def initialize(parent, args = nil)
      super(parent)
      @data_engine_script = parent
      connect(@data_engine_script.dataEngine, SIGNAL("sourceAdded(QString)"), self, SIGNAL("sourceAdded(QString)"))
      connect(@data_engine_script.dataEngine, SIGNAL("sourceRemoved(QString)"), self, SIGNAL("sourceRemoved(QString)"))
    end

    def sourceRequestEvent(name)
    end

    def updateSourceEvent(source)
    end

    def setData(*args)
      @data_engine_script.setData(*args)
    end

    def removeAllData(source)
      @data_engine_script.removeAllData(source)
    end

    def removeData(source, key)
      @data_engine_script.removeData(source, key)
    end

    def setMaxSourceCount(limit)
      @data_engine_script.setMaxSourceCount(limit)
    end

    def maxSourceCount=(limit)
      setMaxSourceCount(limit)
    end

    def setMinimumPollingInterval(minimumMs)
      @data_engine_script.setMinimumPollingInterval(minimumMs)
    end

    def minimumPollingInterval=(minimumMs)
      setMinimumPollingInterval(minimumMs)
    end

    def minimumPollingInterval
      @data_engine_script.minimumPollingInterval
    end

    def setPollingInterval(frequency)
      @data_engine_script.setPollingInterval(frequency)
    end

    def pollingInterval=(frequency)
      setPollingInterval(frequency)
    end

    def removeAllSources
      @data_engine_script.removeAllSources
    end
  end
end