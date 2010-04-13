#!/usr/bin/ruby

require 'qtruby4'

module KDE
    
end

module KIO
    module Internal
        def self.init_all_classes
            getClassList.each do |c|
#                 classname = Qt::Internal::normalize_classname(c)
                classname = c
                id = Qt::Internal::findClass(c);
                if classname[0..4] == "KIO::"
                    mod = KIO
                elsif classname[0..3] == "Qt::"
                    mod = Qt
                elsif classname[0..4] == "KDE::"
                    mod = KDE
                elsif classname[0..1] =~ /K[A-Z]/
                    mod = KDE
                    classname = "KDE::" + classname[1..(classname.size-1)]
                elsif classname[0..1] =~ /Q[A-Z]/
                    mod = Qt
                    classname = "Qt::" + classname[1..(classname.size-1)]
                else
                    mod = KIO
                    classname = "KIO::" + classname
                end
                Qt::Internal::insert_pclassid(classname, id)
                Qt::Internal::cpp_names[classname] = c
                klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, mod) \
                                                    : Qt::Internal::create_qt_class(classname, mod)
                Qt::Internal::classes[classname] = klass unless klass.nil?
            end
        end
    end
end
