require 'Qt'

class TicTacButton < Qt::PushButton

	attr_accessor :type

	Blank, Circle, Cross = 0, 1, 2
	
	def initialize(p,n='')
		#super(0,0,0,0)
		super(p,n)
		@type = Blank
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

		grid = Qt::GridLayout.new(self, n, n, 4)
		p = Qt::Palette.new(Qt::blue)

		for i in (0..n-1)
			ttb = TicTacButton.new(self)
			#ttb = Qt::PushButton.new(self)
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
			#if @buttons[i].type != @btArray[i]
			#	@buttons[i].type = @btArray[i]
			#end
			if @btArray[i] == TicTacButton::Blank
				@buttons[i].setEnabled(true)
			else
				@buttons[i].setEnabled(true)
			end
		end
	end

	def checkBoard
		t = 0
		row = 0
		col = 0
		won = false

		for row in 0..nBoard-1
			if won
				break
			end
			t = @buttons[row*@nBoard]
			if (t == Blank)
				next
			end
			col = 1
			while ( (col < @nBoard) && (@buttons[row*@nBoard] == t) )
				col = col + 1
			end
			if (col == @nBoard)
				won = true
			end
		end
		#for col in 
	end

	def computerMove
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
		if (checkBoard == 0)
			computerMove
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
