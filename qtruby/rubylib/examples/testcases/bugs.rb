require 'Qt'

def bug1
   p1 = Qt::Point.new(5,5)
   p1.setX 5 
   p p1
   p3 = p1.dup
   p3.setX 5 
   p p3
end
class Foo < Qt::PushButton
   def initialize
      super
   end
end
def bug2
   a = Qt::Application.new(ARGV)
   hello = Foo.new
   hello.resize(100, 30)
   a.setMainWidget(hello)
   hello.show()
   a.exec()
end
bug2
