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
end

module PlasmaScripting
  class Applet < Qt::Object
    attr_accessor :applet_script

    def initialize(parent, args = nil)
      super(parent)
    end

    def paintInterface(painter, option, contentsRect)
    end

    def size
      @applet_script.size
    end

    def constraintsEvent(constraints)
    end

    def contextualActions
      return []
    end

    def showConfigurationInterface
    end

    def dataEngine(engine)
      @applet_script.dataEngine(engine)
    end

    def package
      @applet_script.package
    end
  end

  class DataEngine < Qt::Object
    def initialize(parent, args)
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