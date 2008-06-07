=begin
/***************************************************************************
                          okular.rb  -  Okular ruby client lib
                             -------------------
    begin                : Thurs May 29 2008
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

module Okular
  module Internal
    def self.init_all_classes
#      Qt::Internal::add_normalize_proc(Proc.new do |classname|
#        if classname =~ /^Okular/
#          now = classname.sub(/^Okular?(?=[A-Z])/,'Okular::')
#        end
#        now
#      end)
      getClassList.each do |c|
        classname = Qt::Internal::normalize_classname(c)
        id = Qt::Internal::findClass(c);
        Qt::Internal::insert_pclassid(classname, id)
        Qt::Internal::cpp_names[classname] = c
        klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, Okular) \
                                           : Qt::Internal::create_qt_class(classname, Okular)
        Qt::Internal::classes[classname] = klass unless klass.nil?
      end
    end
  end
end
