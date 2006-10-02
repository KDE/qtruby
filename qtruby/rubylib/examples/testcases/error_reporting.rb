require 'Qt'

#### CRASH ###
# param mismatch?
class Bug1 < Qt::PushButton
   def initialize(*k)
      super(*k)
   end
   def Bug1.test
      a = Qt::Application.new(ARGV)
      w = Qt::VBox.new
      hello = Bug1.new(a)
      hello.resize(100, 30)
      a.setMainWidget(w)
      hello.show()
      a.exec()
   end
end
#Bug1.test


#### MORE DEBUG INFO NEEDED ###
# missing method
class Bug2 < Qt::VBox
   def initialize(*k)
      super(*k)
   end
   def Bug2.test
      a = Qt::Application.new(ARGV)
      w = Bug2.new
      a.setMainWidget(w)
      w.show2()
      a.exec()
   end
end
#Bug2.test


#### MORE DEBUG INFO NEEDED ###
# missing prototype
class Bug2a < Qt::VBox
   def initialize(*k)
      super(*k)
   end
   def Bug2a.test
      a = Qt::Application.new(ARGV)
      w = Bug2a.new
      a.setMainWidget(w)
      w.show(p)
      a.exec()
   end
end
Bug2a.test


#### FIXED ###
# no such constructor for PushButton
class Bug3 < Qt::PushButton
   def initialize
      super
   end
   def Bug3.test
      a = Qt::Application.new(ARGV)
      hello = Bug3.new
      hello.resize(100, 30)
      a.setMainWidget(hello)
      hello.show()
      a.exec()
   end
end
#Bug3.test


#### FIXED ###
# no *class* variable/method resize in PushButton
class Bug4 < Qt::PushButton
   def initialize
      super
   end
   def Bug4.test
      hello = Bug4
      hello.resize(100, 30)
   end
end
#Bug4.test
