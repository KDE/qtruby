require 'Qt'


#### FIXED ###
class Bug2 < Qt::PushButton
   def initialize
      super
   end
   def Bug2.test
      a = Qt::Application.new(ARGV)
      hello = Bug2.new
      hello.resize(100, 30)
      a.setMainWidget(hello)
      hello.show()
      a.exec()
   end
end
#Bug2.test


#### TODO ###
# crash on invalid syntax bug
class Bug3 < Qt::PushButton
   def initialize
      super
   end
   def Bug3.test
      hello = Bug3
      hello.resize(100, 30)
   end
end
Bug3.test
