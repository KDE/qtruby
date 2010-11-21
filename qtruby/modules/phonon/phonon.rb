#!/usr/bin/ruby

module Phonon
    module Internal
        def self.init_all_classes
#            Qt::Internal::add_normalize_proc(Proc.new do |classname|
#                if classname =~ /^Q/
#                    now = classname.sub(/^Qt?(?=[A-Z])/,'Qt::')
#                end
#                now
#            end)
            getClassList.each do |c|
                classname = Qt::Internal::normalize_classname(c)
                id = Qt::Internal::findClass(c);
                Qt::Internal::insert_pclassid(classname, id)
                Qt::Internal::cpp_names[classname] = c
                klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, Phonon) \
                                                    : Qt::Internal::create_qt_class(classname, Phonon)
                Qt::Internal::classes[classname] = klass unless klass.nil?
            end
        end
    end
end
