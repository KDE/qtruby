require 'lcdrange.rb'
require 'cannon.rb'

class GameBoard < Qt::Widget

	slots 'fire()', 'hit()', 'missed()', 'newGame()'

	def initialize()
		super
    	quit = Qt::PushButton.new('&Quit', self, 'quit')
    	quit.setFont(Qt::Font.new('Times', 18, Qt::Font.Bold))
    
		connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
    
		angle = LCDRange.new( 'ANGLE', self, 'angle' )
		angle.setRange( 5, 70 )
		
		force  = LCDRange.new( 'FORCE', self, 'force' )
		force.setRange( 10, 50 )
		
		box = Qt::VBox.new( self, 'cannonFrame' )
        box.setFrameStyle( Qt::Frame.WinPanel | Qt::Frame.Sunken )
		@cannonField = CannonField.new( box, 'cannonField' )

		connect( angle, SIGNAL('valueChanged(int)'),
				@cannonField, SLOT('setAngle(int)') )
		connect( @cannonField, SIGNAL('angleChanged(int)'),
				angle, SLOT('setValue(int)') )

		connect( force, SIGNAL('valueChanged(int)'),
				@cannonField, SLOT('setForce(int)') )
		connect( @cannonField, SIGNAL('forceChanged(int)'),
				force, SLOT('setValue(int)') )
		
		connect( @cannonField, SIGNAL('hit()'),
					self, SLOT('hit()') )
		connect( @cannonField, SIGNAL('missed()'),
					self, SLOT('missed()') )
				
		shoot = Qt::PushButton.new( '&Shoot', self, 'shoot' )
		shoot.setFont( Qt::Font.new( 'Times', 18, Qt::Font.Bold ) )

		connect( shoot, SIGNAL('clicked()'), SLOT('fire()') )
		connect( @cannonField, SIGNAL('canShoot(bool)'),
					shoot, SLOT('setEnabled(bool)') )
								
		restart = Qt::PushButton.new( '&New Game', self, 'newgame' )
		restart.setFont( Qt::Font.new( 'Times', 18, Qt::Font.Bold ) )

		connect( restart, SIGNAL('clicked()'), self, SLOT('newGame()') )

		@hits = Qt::LCDNumber.new( 2, self, 'hits' )
		@shotsLeft = Qt::LCDNumber.new( 2, self, 'shotsleft' )
		hitsL = Qt::Label.new( 'HITS', self, 'hitsLabel' )
		shotsLeftL = Qt::Label.new( 'SHOTS LEFT', self, 'shotsleftLabel' )
				
		accel = Qt::Accel.new( self )
        accel.connectItem( accel.insertItem( Qt::KeySequence.new(Qt.Key_Enter) ),
                            self, SLOT('fire()') )
        accel.connectItem( accel.insertItem( Qt::KeySequence.new(Qt.Key_Return) ),
                            self, SLOT('fire()') )
		accel.connectItem( accel.insertItem( Qt::KeySequence.new('Qt.CTRL+Key_Q') ),
							$qApp, SLOT('quit()') )
							 		
		grid = Qt::GridLayout.new( self, 2, 2, 10 )
		grid.addWidget( quit, 0, 0 )
		grid.addWidget( box, 1, 1)
		grid.setColStretch( 1, 10 )

		leftBox = Qt::VBoxLayout.new()
		grid.addLayout( leftBox, 1, 0 )
		leftBox.addWidget( angle )
		leftBox.addWidget( force )
	
		topBox = Qt::HBoxLayout.new()
		grid.addLayout( topBox, 0, 1 )
		topBox.addWidget( shoot )
		topBox.addWidget( @hits )
		topBox.addWidget( hitsL )
		topBox.addWidget( @shotsLeft )
		topBox.addWidget( shotsLeftL )
		topBox.addStretch( 1 )
		topBox.addWidget( restart )
	
		angle.setValue( 60 )
		force.setValue( 25 )
		angle.setFocus()
		
		newGame()
	end
	
	def fire()
		if @cannonField.gameOver() || @cannonField.isShooting()
			return
		end
		@shotsLeft.display( @shotsLeft.intValue() - 1 )
		@cannonField.shoot()
	end

	def hit()
		@hits.display( @hits.intValue() + 1 )
		if @shotsLeft.intValue() == 0
			@cannonField.setGameOver()
		else
			@cannonField.newTarget()
		end
	end

	def missed()
		if @shotsLeft.intValue() == 0
			@cannonField.setGameOver()
		end
	end

	def newGame()
		@shotsLeft.display( 15.0 )
		@hits.display( 0 )
		@cannonField.restartGame()
		@cannonField.newTarget()
	end
end
