class OptionsForm

slots 'languageChange()'


def initialize(*k)
    super(*k)

    if name.nil?
    	setName("OptionsForm")
    end

    @OptionsFormLayout = Qt::VBoxLayout.new(self, 11, 6, '@OptionsFormLayout')

    @GroupBox2 = Qt::GroupBox.new(self, "@GroupBox2")
    @GroupBox2.setColumnLayout( 0, Qt::Vertical )
    @GroupBox2.layout().setSpacing(6)
    @GroupBox2.layout().setMargin(11)
    @GroupBox2Layout = Qt::HBoxLayout.new(@GroupBox2.layout() )
    @GroupBox2Layout.setAlignment( AlignTop )

    @webCheckBox = Qt::CheckBox.new(@GroupBox2, "@webCheckBox")
    @webCheckBox.setChecked( true )
    @GroupBox2Layout.addWidget(@webCheckBox)
    @OptionsFormLayout.addWidget(@GroupBox2)

    @ButtonGroup1 = Qt::ButtonGroup.new(self, "@ButtonGroup1")
    @ButtonGroup1.setColumnLayout( 0, Qt::Vertical )
    @ButtonGroup1.layout().setSpacing(6)
    @ButtonGroup1.layout().setMargin(11)
    @ButtonGroup1Layout = Qt::VBoxLayout.new(@ButtonGroup1.layout() )
    @ButtonGroup1Layout.setAlignment( AlignTop )

    @hexRadioButton = Qt::RadioButton.new(@ButtonGroup1, "@hexRadioButton")
    @hexRadioButton.setChecked( true )
    @ButtonGroup1Layout.addWidget(@hexRadioButton)

    @nameRadioButton = Qt::RadioButton.new(@ButtonGroup1, "@nameRadioButton")
    @ButtonGroup1Layout.addWidget(@nameRadioButton)

    @rgbRadioButton = Qt::RadioButton.new(@ButtonGroup1, "@rgbRadioButton")
    @ButtonGroup1Layout.addWidget(@rgbRadioButton)
    @OptionsFormLayout.addWidget(@ButtonGroup1)

    @Layout5 = Qt::HBoxLayout.new(nil, 0, 6, '@Layout5')
    @Spacer2 = Qt::SpacerItem.new(0, 10, Qt::SizePolicy::Expanding, Qt::SizePolicy::Minimum)
    @Layout5.addItem(@Spacer2)

    @okPushButton = Qt::PushButton.new(self, "@okPushButton")
    @okPushButton.setDefault( true )
    @Layout5.addWidget(@okPushButton)

    @cancelPushButton = Qt::PushButton.new(self, "@cancelPushButton")
    @Layout5.addWidget(@cancelPushButton)
    @OptionsFormLayout.addLayout(@Layout5)
    languageChange()
    resize( Qt::Size.new(306, 226).expandedTo(minimumSizeHint()) )
    clearWState( WState_Polished )

    Qt::Object.connect(@okPushButton, SIGNAL("clicked()"), self, SLOT("accept()") )
    Qt::Object.connect(@cancelPushButton, SIGNAL("clicked()"), self, SLOT("reject()") )
end

#
#  Sets the strings of the subwidgets using the current
#  language.
#
def languageChange()
    setCaption(trUtf8("Color Tool -- Options"))
    @GroupBox2.setTitle( trUtf8("Table View") )
    @webCheckBox.setText( trUtf8("Indicate &Web Colors") )
    @ButtonGroup1.setTitle( trUtf8("Copy to Clipboard As") )
    @hexRadioButton.setText( trUtf8("&Hex, e.g. #AB347F") )
    @nameRadioButton.setText( trUtf8("&Name, e.g. light blue") )
    @rgbRadioButton.setText( trUtf8("&RGB, e.g. 51,255,102") )
    @okPushButton.setText( trUtf8("OK") )
    @cancelPushButton.setText( trUtf8("Cancel") )
end
protected :languageChange

attr_reader :webCheckBox,
			:hexRadioButton, 
			:nameRadioButton, 
			:rgbRadioButton,
			:okPushButton,
			:cancelPushButton
end
