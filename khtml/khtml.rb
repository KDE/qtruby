#!/usr/bin/ruby

module DOM
  module Internal
    def self.init_all_classes
      getClassList.each do |c|
        classname = Qt::Internal::normalize_classname(c)
        id = Qt::Internal::findClass(c);
        Qt::Internal::insert_pclassid(classname, id)
        Qt::Internal::cpp_names[classname] = c
        if classname =~ /^DOM/
          m = DOM
        elsif classname =~ /^KParts/
          m = KParts
        else
          m = KDE
        end
        klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, m) \
                                           : Qt::Internal::create_qt_class(classname, m)
        Qt::Internal::classes[classname] = klass unless klass.nil?
      end
    end
  end
end
