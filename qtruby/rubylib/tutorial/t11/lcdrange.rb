require 'Qt'

class LCDRange < Qt::VBox
    signals 'valueChanged(int)'
    slots 'setValue(int)', 'setRange(int, int)'

    def initialize(parent, name)
        super
        lcd = Qt::LCDNumber.new(2, self, 'lcd')
        @slider = Qt::Slider.new(Qt::VBox::Horizontal, self, 'slider')
        @slider.setRange(0, 99)
        @slider.setValue(0)
        connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
        connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))
        setFocusProxy(@slider)
    end

    def value()
        @slider.value()
    end

    def setValue( value )
        @slider.setValue( value )
    end
    
    def setRange( minVal, maxVal )
		if minVal < 0 || maxVal > 99 || minVal > maxVal
      		Qt.qWarning( "LCDRange::setRange(#{minVal},#{maxVal})\n" +
               		"\tRange must be 0..99\n" +
               		"\tand minVal must not be greater than maxVal" )
			return
		end
        @slider.setRange( minVal, maxVal )
    end
end
