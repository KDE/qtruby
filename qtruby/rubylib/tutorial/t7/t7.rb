#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'
require 'lcdrange.rb'

class MyWidget < Qt::VBox

def initialize()
	super
    quit = Qt::PushButton.new('Quit', self, 'quit')
    quit.setFont(Qt::Font.new('Times', 18, Qt::Font::Bold))
    
	connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
	grid = Qt::Grid.new( 4, self )

	previous = nil	
	for c in 0..3
		for r in 0..3
			lr = LCDRange.new(grid)
			if previous != nil
				connect( lr, SIGNAL('valueChanged(int)'),
						previous, SLOT('setValue(int)') )
			end
			previous = lr
		end
	end
end

end    

a = Qt::Application.new(ARGV)

w = MyWidget.new
a.setMainWidget(w)
w.show
a.exec
