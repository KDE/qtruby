#!/usr/bin/ruby -w
require 'Qt'

class LCDRange < Qt::VBox
	signals "valueChanged(int)"
	slots "setValue(int)"

	def initialize(grid)
		super
		lcd = Qt::LCDNumber.new(2, self, "lcd")
	    @slider = Qt::Slider.new(Qt::VBox.Horizontal, self, "slider")
	    @slider.setRange(0, 99)
	    @slider.setValue(0)
		connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
		connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))
	end

	def value()
    	@slider.value()
	end

	def setValue( value )
    	@slider.setValue( value )
	end
end
