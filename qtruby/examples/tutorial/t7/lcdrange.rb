#!/usr/bin/ruby -w
require 'Qt'

class LCDRange < Qt::Widget
    signals 'valueChanged(int)'
    slots 'setValue(int)'

    def initialize(parent = nil)
        super
        lcd = Qt::LCDNumber.new(2)

        @slider = Qt::Slider.new(Qt::Horizontal)
        @slider.setRange(0, 99)
        @slider.setValue(0)

        connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
        connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))
   
        layout = Qt::VBoxLayout.new
        layout.addWidget(lcd)
        layout.addWidget(@slider)
        setLayout(layout)
    end

    def value()
        @slider.value()
    end

    def setValue( value )
        @slider.setValue( value )
    end
end
