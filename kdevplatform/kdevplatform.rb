=begin
/***************************************************************************
                          kdevplatform.rb  -  KDevelop ruby client lib
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

module Sublime
end

module KDevelop
  module Internal
    def self.init_all_classes
      getClassList.each do |c|
        classname = Qt::Internal::normalize_classname(c)
        id = Qt::Internal::findClass(c);
        Qt::Internal::insert_pclassid(classname, id)
        Qt::Internal::cpp_names[classname] = c
        if classname =~ /^KDevelop/
          m = KDevelop
        elsif classname =~ /^Sublime/
          m = Sublime
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
