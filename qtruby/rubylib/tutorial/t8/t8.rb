#!/usr/bin/env ruby -w
require 'Qt'
require "lcdrange.rb"
require "cannon.rb"

class MyWidget < Qt::Widget
	def initialize()
		super
    	quit = Qt::PushButton.new("Quit", self, "quit")
    	quit.setFont(Qt::Font.new("Times", 18, Qt::Font.Bold))
    
		connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
    
		angle = LCDRange.new( self, "angle" )
		angle.setRange( 5, 70 )

		cannonField = CannonField.new( self, "cannonField" )

		connect( angle, SIGNAL('valueChanged(int)'),
				cannonField, SLOT('setAngle(int)') )
		connect( cannonField, SIGNAL('angleChanged(int)'),
				angle, SLOT('setValue(int)') )
		grid = Qt::GridLayout.new( self, 2, 2, 10 )
		# 2x2, 10 pixel border

		grid.addWidget( quit, 0, 0 )
		grid.addWidget( angle, 1, 0, Qt.AlignTop )
		grid.addWidget( cannonField, 1, 1 )
		grid.setColStretch( 1, 10 )

		angle.setValue( 60 )
		angle.setFocus()
	end
end    

a = Qt::Application.new(ARGV)

w = MyWidget.new
w.setGeometry( 100, 100, 500, 355 )
a.setMainWidget(w)
w.show
a.exec
