require 'Qt'


#### TODO ###
def bug1
   p1 = Qt::Point.new(5,5)
   p1.setX 5 
   p p1
   p3 = p1.dup
   p3.setX 5 
   p p3
end
#bug1

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
class Bug3 < Qt::PushButton
   def initialize
      super
   end
   def Bug3.test
      hello = Bug3
      hello.resize(100, 30)
   end
end
#Bug3.test


#### TODO ###
def bug3
    a = Qt::Application.new(ARGV)
    @file = Qt::PopupMenu.new
    @file.insertSeparator
    @file.insertItem("Quit", $qApp, SLOT('quit()'))
    @file.exec
end
bug3
