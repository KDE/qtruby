#!/usr/bin/env ruby -w
require 'Qt'

class MyWidget < Qt::VBox

def initialize()
   super
    quit = Qt::PushButton.new("Quit", self, "quit")
    quit.setFont(Qt::Font.new("Times", 18, Qt::Font.Bold))
    
	connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
    
	lcd = Qt::LCDNumber.new(2, self, "lcd")

    slider = Qt::Slider.new(Qt::VBox.Horizontal, self, "slider")
    slider.setRange(0, 99)
    slider.setValue(0)

    lcd.connect(slider, SIGNAL('valueChanged(int)'), SLOT('display(int)'))
end

end

a = Qt::Application.new(ARGV)

w = MyWidget.new
a.setMainWidget(w)
w.show
a.exec
