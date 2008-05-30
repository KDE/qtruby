#!/usr/bin/ruby

require 'Qt4'

module Qsci
    module Internal
        def self.init_all_classes
            Qt::Internal::add_normalize_proc(Proc.new do |classname|
                if classname =~ /^Qsci/
                    now = classname.sub(/^Qsci?(?=[A-Z])/,'Qsci::')
                end
                now
            end)
            getClassList.each do |c|
                classname = Qt::Internal::normalize_classname(c)
                id = Qt::Internal::findClass(c);
                Qt::Internal::insert_pclassid(classname, id)
                Qt::Internal::cpp_names[classname] = c
                klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, Qsci) \
                                                    : Qt::Internal::create_qt_class(classname, Qsci)
                Qt::Internal::classes[classname] = klass unless klass.nil?
            end
        end
    end
end
