require 'Qt'


#### TODO ###
# dup of qobject crash
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
#Bug3.test


#### FIXED ###
def bug3
    a = Qt::Application.new(ARGV)
    @file = Qt::PopupMenu.new
    @file.insertSeparator
    Qt::debug_level = Qt::DebugLevel::High
    p $qApp
    @file.insertItem("Quit", $qApp, SLOT('quit()'))
    @file.exec
end
#bug3


class CPUWaster < Qt::Widget
    def initialize(*k)
        super(*k)
    end
    def draw
	painter = Qt::Painter.new(self)
	0.upto(1000) { |i|
	    cw, ch = width, height
	    c = Qt::Color.new(rand(255), rand(255), rand(255))
	    x = rand(cw - 8)
	    y = rand(cw - 8)
	    w = rand(cw - x)
	    h = rand(cw - y)
	    brush = Qt::Brush.new(c)
	    brush.setStyle(Qt::Dense6Pattern)
	    painter.fillRect(x, y, w, h, brush)
	}
    end
end
def bug4
   Qt::Application.new(ARGV)
   w = CPUWaster.new
   w.show
   w.draw
end
bug4
