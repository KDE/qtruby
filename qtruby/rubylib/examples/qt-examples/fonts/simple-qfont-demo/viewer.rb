class Viewer < Qt::Widget
	slots 'setDefault()', 'setSansSerif()', 'setItalics()'

	def initialize
		super

		setFontSubstitutions

		greeting_en = "Hello"

		@greetings = Qt::TextView.new(self, 'textview')
		@greetings.setText(greeting_en)

		@fontInfo = Qt::TextView.new(self, 'fontinfo')

		setDefault

		@defaultButton = Qt::PushButton.new('Default', self, 'pushbutton1')
		@defaultButton.setFont(Qt::Font.new('times'))
		connect(@defaultButton, SIGNAL('clicked()'), self, SLOT('setDefault()'))


		@sansSerifButton = Qt::PushButton.new('Sans Serif', self, 'pushbutton2')
		@sansSerifButton.setFont(Qt::Font.new('Helvetica', 12))
		connect(@sansSerifButton, SIGNAL('clicked()'), self, SLOT('setSansSerif()'))

		@italicsButton = Qt::PushButton.new('Italics', self, 'pushbutton1')
		@italicsButton.setFont(Qt::Font.new('lucida', 12, Qt::Font.Bold, true))
		connect(@italicsButton, SIGNAL('clicked()'), self, SLOT('setItalics()'))

		layout
	end

	def setDefault
		font = Qt::Font.new('Bavaria')
		font.setPointSize(24)
		font.setWeight(Qt::Font.Bold)
		font.setUnderline(true)

		@greetings.setFont(font)
		showFontInfo(font)
	end

	def setSansSerif
		font = Qt::Font.new('Newyork', 18)
		font.setStyleHint(Qt::Font.SansSerif)

		@greetings.setFont(font)
		showFontInfo(font)
	end

	def setItalics
		font = Qt::Font.new('Tokyo')
		font.setPointSize(32)
		font.setWeight(Qt::Font.Bold)
		font.setItalic(true)

		@greetings.setFont(font)
		showFontInfo(font)
	end

	def setFontSubstitutions
		substitutes = Array.new

		substitutes.push('Times')
		substitutes.push('Mincho')
		substitutes.push('Arabic Newspaper')
		substitutes.push('crox')

		Qt::Font.insertSubstitutions('Bavaria', substitutes)
		Qt::Font.insertSubstitutions('Tokyo', 'Lucida')
	end

	def layout
		textViewContainer = Qt::HBoxLayout.new
		textViewContainer.addWidget(@greetings)
		textViewContainer.addWidget(@fontInfo)

		buttonContainer = Qt::HBoxLayout.new
		buttonContainer.addWidget(@defaultButton)
		buttonContainer.addWidget(@sansSerifButton)
		buttonContainer.addWidget(@italicsButton)

		maxButtonHeight = @defaultButton.height

		if (@sansSerifButton.height > maxButtonHeight)
			maxButtonHeight = @sansSerifButton.height
		end

		if (@italicsButton.height > maxButtonHeight)
			maxButtonHeight = @italicsButton.height
		end

		@defaultButton.setFixedHeight(maxButtonHeight)
		@sansSerifButton.setFixedHeight(maxButtonHeight)
		@italicsButton.setFixedHeight(maxButtonHeight)

		container = Qt::VBoxLayout.new(self)
		container.addLayout(textViewContainer)
		container.addLayout(buttonContainer)

		resize(700, 250)
	end

	def showFontInfo (font)
		info = Qt::FontInfo.new(font)

		messageText =
			'Font requested: "' +
			font.family + '" ' +
			font.pointSize.to_s + 'pt<BR>' +
			'Font used: "' +
			info.family + '" ' +
			info.pointSize.to_s + 'pt<P>'

		substitutions = Qt::Font.substitutes(font.family)

		unless substitutions.size == 0
			messageText = messageText + 'The following substitutions exist for ' +
				font.family + ':<UL>'
			substitutions.each {|x|
				messageText = messageText + '<LI>"' + x + '"'
			}
			messageText = messageText + '</UL>'
		else
			messageText = messageText + 'No substitutions exist for ' + font.family + '.'
		end

		@fontInfo.setText(messageText)
	end
end
