#!/usr/bin/ruby -w
require 'Qt'

class LCDRange < Qt::VBox

def initialize(grid)
	lcd = Qt::LCDNumber.new(2, self, "lcd")

    slider = Qt::Slider.new(VBox.Horizontal, self, "slider")
    slider.setRange(0, 99)
    slider.setValue(0)

    lcd.connect(slider, SIGNAL('valueChanged(int)'), SLOT('display(int)'))
end

end

class MyWidget < Qt::VBox

def initialize()
    quit = Qt::PushButton.new("Quit", self, "quit")
    quit.setFont(Qt::Font.new("Times", 18, Qt::Font.Bold))
    
	connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
	grid = Qt::Grid.new( 4, self )
	
	for c in 0..3
		for r in 0..3
			LCDRange.new(grid)
		end
	end
end

end    

a = Qt::Application.new(ARGV)

w = MyWidget.new
a.setMainWidget(w)
w.show
a.exec
