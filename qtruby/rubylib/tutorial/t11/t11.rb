#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'
require 'lcdrange.rb'
require 'cannon.rb'

class MyWidget < Qt::Widget
	def initialize()
		super
    	quit = Qt::PushButton.new('Quit', self, 'quit')
    	quit.setFont(Qt::Font.new('Times', 18, Qt::Font.Bold))
    
		connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
    
		angle = LCDRange.new( self, 'angle' )
		angle.setRange( 5, 70 )
		
		force  = LCDRange.new( self, 'force' )
		force.setRange( 10, 50 )
		
		cannonField = CannonField.new( self, 'cannonField' )

		connect( angle, SIGNAL('valueChanged(int)'),
				cannonField, SLOT('setAngle(int)') )
		connect( cannonField, SIGNAL('angleChanged(int)'),
				angle, SLOT('setValue(int)') )

		connect( force, SIGNAL('valueChanged(int)'),
				cannonField, SLOT('setForce(int)') )
		connect( cannonField, SIGNAL('forceChanged(int)'),
				force, SLOT('setValue(int)') )
		
		shoot = Qt::PushButton.new( '&Shoot', self, 'shoot' )
		shoot.setFont( Qt::Font.new( 'Times', 18, Qt::Font.Bold ) )

		connect( shoot, SIGNAL('clicked()'), cannonField, SLOT('shoot()') )
			
		grid = Qt::GridLayout.new( self, 2, 2, 10 )
		grid.addWidget( quit, 0, 0 )
		grid.addWidget( cannonField, 1, 1 )
		grid.setColStretch( 1, 10 )

		leftBox = Qt::VBoxLayout.new()
		grid.addLayout( leftBox, 1, 0 )
		leftBox.addWidget( angle )
		leftBox.addWidget( force )
	
		topBox = Qt::HBoxLayout.new()
		grid.addLayout( topBox, 0, 1 )
		topBox.addWidget( shoot )
		topBox.addStretch( 1 )
	
		angle.setValue( 60 )
		force.setValue( 25 )
		angle.setFocus()
	end
end    

Qt::Application.setColorSpec( Qt::Application.CustomColor )
a = Qt::Application.new(ARGV)

w = MyWidget.new
w.setGeometry( 100, 100, 500, 355 )
a.setMainWidget(w)
w.show
a.exec
