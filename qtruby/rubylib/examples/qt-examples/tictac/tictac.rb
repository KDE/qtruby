require 'Qt'

class TicTacButton < Qt::PushButton

	attr_accessor :type

	Blank, Circle, Cross = 0, 1, 2
	
	def initialize(p)
		super(p)
		@type = Blank
	end

	def drawButtonLabel(p)
		r = rect()
		p.setPen( Qt::Pen.new( Qt::white,2 ) ) #set fat pen
		if (@type == Circle)
			p.drawEllipse( r.left()+4, r.top()+4, r.width()-8, r.height()-8 )
		elsif (@type == Cross)
			#draw cross
			x1 = r.topLeft.x+4
			x2 = r.bottomRight.x-4
			y1 = r.topLeft.y+4
			y2 = r.bottomRight.y-4
			p.drawLine( x1, y1, x2, y2)

			x1 = r.bottomLeft.x+4
			x2 = r.topRight.x+4
			y1 = r.bottomLeft.y-4
			y2 = r.topRight.y-4
			p.drawLine( x1, y1, x2, y2)
		end
		super(p)
	   end
		   

end

class TicTacGameBoard < Qt::Widget
	signals 'finished()'
	slots 'buttonClicked()'

	Init, HumansTurn, HumanWon, ComputerWon, NobodyWon = 0, 1, 2, 3, 4
	
	attr_accessor :state, :computer_starts
	
	def initialize (n, parent)
		super(parent)
		@state = Init
		@nBoard = n
		n = n*n
		@computer_starts = false
		@buttons = Array.new(n)
		@btArray = Array.new(n)
		@btArray.each{|bt|
			bt = TicTacButton::Blank
		}

		grid = Qt::GridLayout.new(self, n, n, 4)
		p = Qt::Palette.new(Qt::blue)

		for i in (0..n-1)
			ttb = TicTacButton.new(self)
			ttb.setPalette(p)
			ttb.setEnabled(false)
			connect(ttb, SIGNAL('clicked()'), self, SLOT('buttonClicked()'))
			grid.addWidget(ttb, i % @nBoard, i / @nBoard)
			@buttons[i] = ttb
			@btArray[i] = TicTacButton::Blank
		end
	end

	def newGame
		@state = HumansTurn
		for i in 0..(@nBoard*@nBoard)-1
			@btArray[i] = TicTacButton::Blank
		end
		if @computer_starts == true
			computerMove
		else
			updateButtons
		end
	end

	def updateButtons
		for i in 0..(@nBoard*@nBoard)-1
			if @buttons[i].type != @btArray[i]
				@buttons[i].type = @btArray[i]
			end
			if @buttons[i].type == TicTacButton::Blank
				@buttons[i].setEnabled(true)
			else
				@buttons[i].setEnabled(false)
			end
		end
	end

	def checkBoard
		t = 0
		row = 0
		col = 0
		won = false

		for row in 0..@nBoard-1
			if won == true
				break
			end
			t = @btArray[row*@nBoard]
			if (t == TicTacButton::Blank)
				next
			end
			col = 1
			while ( (col < @nBoard) && (@btArray[row*@nBoard+col] == t) )
				col = col + 1
			end
			if (col == @nBoard)
				won = true
			end
		end
		for col in 0..@nBoard-1
			if won == true
				break
			end
			t = @btArray[col]
			if (t == TicTacButton::Blank)
				next
			end
			while ( (row < @nBoard) && (@btArray[row*@nBoard+col] == t) )
				row = row +1
			end
			if (row == @nBoard)
				won = true
			end
		end
		if (won == false)	# check diagonal top left
			t = @btArray[0]	# to bottom right
			if (t != TicTacButton::Blank)
				i = 1;
				while (i<@nBoard && (@btArray[i*@nBoard+i] == t))
					i = i +1
				end
				if (i == @nBoard)
					won = true
				end
			end
		end

		if (won == false)	# check diagonal bottom left
			j = @nBoard-1	# to top right
			i = 0;
			t = @btArray[i+j*@nBoard];
			if (t != TicTacButton::Blank)
				i = i +1
				j = j - 1
				while ( (i<@nBoard) && (@btArray[i+j*@nBoard] == t) )
					i = i + 1
					j = j -1
				end
				if (i == @nBoard)
					won = true
				end
			end
		end
		
		if (won == false)
			t = 0
		end
		return t
	end

	def computerMove
		numButtons = @nBoard*@nBoard
		altv = Array.new
		stopHuman = -1

		for i in 0..numButtons-1				# try all positions
			if @btArray[i] != TicTacButton::Blank		# already a piece there
				next
			end

			@btArray[i] = TicTacButton::Cross		# test if computer wins
			s = checkBoard
			if (s == @btArray[i])				# computer will win
				@state = ComputerWon
				stopHuman = -1
				break
			end

			@btArray[i] = TicTacButton::Circle		# test if human wins
			if (checkBoard == @btArray[i])			# oops...
				stopHuman = i				# remember position
				@btArray[i] = TicTacButton::Blank	# restore button
				next					# computer still might win
			end
			@btArray[i] = TicTacButton::Blank;		# restore button
			altv.push[i]					# remember alternative
		end

		if (stopHuman >= 0)					# must stop human from winning
			@btArray[stopHuman] = TicTacButton::Cross
		elsif (i == numButtons)					# tried all alternatives
			if (altv.size > 0)				# set random piece
				@btArray[altv[rand()%(altv.size+5)]] = TicTacButton::Cross
			end
			if ((altv.size-1) == 0)				# no more blanks
				@state = NobodyWon
				emit finished()
			end
		end
		updateButtons						# update buttons
	end

	def buttonClicked
		unless @state == HumansTurn
			return
		end

		b = nil
		at = nil
		for i in 0..@buttons.size
			if @buttons[i].id == sender.id
				b = @buttons[i]
				at = i
				break
			end
		end
		if @btArray[at] == TicTacButton::Blank
			@btArray[at] = TicTacButton::Circle
			updateButtons
		end

		s = checkBoard
		case s
		when TicTacButton::Blank
			computerMove
		when TicTacButton::Circle
			@state = HumanWon
			emit finished()
		when TicTacButton::Cross
			@state = ComputerWon
			emit finished()
		end
	end

end

class TicTacToe < Qt::Widget
	slots 'newGameClicked()', 'gameOver()'

	def initialize (boardSize)
		super()

		l = Qt::VBoxLayout.new(self, 6)

		@state_msg = [
			'Click Play to start',
			'Make your move',
			'You won!',
			'Computer won!',
			'It\'s a draw']
		
		# Create a message label
		@message = Qt::Label.new(self)
		@message.setFrameStyle (Qt::Frame.WinPanel | Qt::Frame.Sunken)
		@message.setAlignment(Qt::AlignCenter)
		l.addWidget(@message)

		# Create the game board and connect the signal finished()
		# to this/self gameOver() slot
		@board = TicTacGameBoard.new(boardSize, self)
		connect(@board, SIGNAL('finished()'), self, SLOT('gameOver()'));
		l.addWidget(@board)

		# Create a horizontal frame line
		line = Qt::Frame.new(self)
		line.setFrameStyle(Qt::Frame.HLine|Qt::Frame.Sunken)
		l.addWidget(line)

		# Create the combo box for deciding who should start
		# and connect its clicked() signals to the buttonClicked() slot
		@whoStarts = Qt::ComboBox.new(self)
		@whoStarts.insertItem('Computer starts')
		@whoStarts.insertItem('Human starts')
		l.addWidget(@whoStarts);

		# Create the push buttons and connect their signals to the right slots
		@newGame = Qt::PushButton.new('Play!', self)
		connect(@newGame, SIGNAL('clicked()'), self, SLOT('newGameClicked()'))
		@quit = Qt::PushButton.new('Quit', self)
		connect(@quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
		b = Qt::HBoxLayout.new
		l.addLayout(b)
		b.addWidget(@newGame)
		b.addWidget(@quit)

		newState()
	end

	def newState
		@message.setText(@state_msg[@board.state])
	end

	def newGameClicked
		if @whoStarts.currentItem == 0
			@board.computer_starts = true
		else
			@board.computer_starts = false
		end
		@board.newGame()
		newState()
	end

	def gameOver
		# Update text box
		newState()
	end
end
