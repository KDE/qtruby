#!/usr/bin/ruby

require 'Qt4'

module Qwt
    module Internal
        def self.init_all_classes
            Qt::Internal::add_normalize_proc(Proc.new do |classname|
                if classname =~ /^Qwt/
                    now = classname.sub(/^Qwt?(?=[A-Z])/,'Qwt::')
                end
                now
            end)
            getClassList.each do |c|
                classname = Qt::Internal::normalize_classname(c)
                id = Qt::Internal::findClass(c);
                Qt::Internal::insert_pclassid(classname, id)
                Qt::Internal::cpp_names[classname] = c
                klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, Qwt) \
                                                    : Qt::Internal::create_qt_class(classname, Qwt)
                Qt::Internal::classes[classname] = klass unless klass.nil?
            end
        end
    end
end
