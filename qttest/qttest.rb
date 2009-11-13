=begin
/***************************************************************************
                          qttest.rb  -  QtTest ruby client lib
                             -------------------
    begin                : 29-10-2008
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

module QtTest
  module Internal
    def self.init_all_classes
      getClassList.each do |c|
        classname = Qt::Internal::normalize_classname(c)
        id = Qt::Internal::findClass(c);
        Qt::Internal::insert_pclassid(classname, id)
        Qt::Internal::cpp_names[classname] = c
        klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, Qt)  : Qt::Internal::create_qt_class(classname, Qt)
        Qt::Internal::classes[classname] = klass unless klass.nil?
      end
    end
  end
end

module Qt
  class Base
    def QVERIFY(statement)
      Qt::Test.qVerify(eval(statement), statement, "", __FILE__, __LINE__)
    end
    
    def QFAIL(message)
      Qt::Test.qFail(message, __FILE__, __LINE__)
    end
    
    def QVERIFY2(statement, description)
      Qt::Test.qVerify(eval(statement), statement, description, __FILE__, __LINE__)
    end
    
    def QCOMPARE(actual, expected)
      Qt::Test.qCompare(eval(actual), eval(expected), actual, expected, __FILE__, __LINE__)    
    end
    
    def QSKIP(statement, mode)
      Qt::Test.qSkip(statement, mode, __FILE__, __LINE__)
    end
  end
end

