class OptionsForm < Qt::Dialog
    slots 'chooseFont()'
    
    attr_reader :chartTypeComboBox, 
	:noRadioButton, 
	:yesRadioButton, 
	:asPercentageRadioButton,
	:decimalPlacesSpinBox
    
    def initialize( parent = nil, name = "options form",
                            modal = false, f = 0 )
        super( parent, name, modal, f )
        setCaption( "Chart -- Options" )
        resize( 320, 290 )
    
        optionsFormLayout = Qt::VBoxLayout.new( self, 11, 6 )
    
        chartTypeLayout = Qt::HBoxLayout.new( nil, 0, 6 )
    
        chartTypeTextLabel = Qt::Label.new( "&Chart Type", self )
        chartTypeLayout.addWidget( chartTypeTextLabel )
    
        @chartTypeComboBox = Qt::ComboBox.new( false, self )
        @chartTypeComboBox.insertItem( Qt::Pixmap.new( "images/options_piechart.xpm" ), "Pie Chart" )
        @chartTypeComboBox.insertItem( Qt::Pixmap.new( "images/options_verticalbarchart.xpm" ),
                                    "Vertical Bar Chart" )
        @chartTypeComboBox.insertItem( Qt::Pixmap.new( "images/options_horizontalbarchart.xpm" ),
                                    "Horizontal Bar Chart" )
        chartTypeLayout.addWidget( @chartTypeComboBox )
        optionsFormLayout.addLayout( chartTypeLayout )
    
        fontLayout = Qt::HBoxLayout.new( nil, 0, 6 )
    
        fontPushButton = Qt::PushButton.new( "&Font...", self )
        fontLayout.addWidget( fontPushButton )
        spacer = Qt::SpacerItem.new( 0, 0, Qt::SizePolicy::Expanding,
                                            Qt::SizePolicy::Minimum )
        fontLayout.addItem( spacer )
    
        @fontTextLabel = Qt::Label.new( self ) # Must be set by caller via setFont()
        fontLayout.addWidget( @fontTextLabel )
        optionsFormLayout.addLayout( fontLayout )
    
        addValuesFrame = Qt::Frame.new( self )
        addValuesFrame.setFrameShape( Qt::Frame::StyledPanel )
        addValuesFrame.setFrameShadow( Qt::Frame::Sunken )
        addValuesFrameLayout = Qt::VBoxLayout.new( addValuesFrame, 11, 6 )
    
        addValuesButtonGroup = Qt::ButtonGroup.new( "Show Values", addValuesFrame )
        addValuesButtonGroup.setColumnLayout(0, Qt::Vertical )
        addValuesButtonGroup.layout().setSpacing( 6 )
        addValuesButtonGroup.layout().setMargin( 11 )
        addValuesButtonGroupLayout = Qt::VBoxLayout.new(
                                            addValuesButtonGroup.layout() )
        addValuesButtonGroupLayout.setAlignment( Qt::AlignTop )
    
        @noRadioButton = Qt::RadioButton.new( "&No", addValuesButtonGroup )
        @noRadioButton.setChecked( true )
        addValuesButtonGroupLayout.addWidget( @noRadioButton )
    
        @yesRadioButton = Qt::RadioButton.new( "&Yes", addValuesButtonGroup )
        addValuesButtonGroupLayout.addWidget( 	@yesRadioButton )
    
        @asPercentageRadioButton = Qt::RadioButton.new( "As &Percentage",
                                                    addValuesButtonGroup )
        addValuesButtonGroupLayout.addWidget( @asPercentageRadioButton )
        addValuesFrameLayout.addWidget( addValuesButtonGroup )
    
        decimalPlacesLayout = Qt::HBoxLayout.new( nil, 0, 6 )
    
        decimalPlacesTextLabel = Qt::Label.new( "&Decimal Places", addValuesFrame )
        decimalPlacesLayout.addWidget( decimalPlacesTextLabel )
    
        @decimalPlacesSpinBox = Qt::SpinBox.new( addValuesFrame )
        @decimalPlacesSpinBox.setMinValue( 0 )
        @decimalPlacesSpinBox.setMaxValue( 9 )
        decimalPlacesLayout.addWidget( @decimalPlacesSpinBox )
    
        addValuesFrameLayout.addLayout( decimalPlacesLayout )
    
        optionsFormLayout.addWidget( addValuesFrame )
    
        buttonsLayout = Qt::HBoxLayout.new( nil, 0, 6 )
        spacer = Qt::SpacerItem.new( 0, 0,
                                Qt::SizePolicy::Expanding, Qt::SizePolicy::Minimum )
        buttonsLayout.addItem( spacer )
    
        okPushButton = Qt::PushButton.new( "OK", self )
        okPushButton.setDefault( true )
        buttonsLayout.addWidget( okPushButton )
    
        cancelPushButton = Qt::PushButton.new( "Cancel", self )
        buttonsLayout.addWidget( cancelPushButton )
        optionsFormLayout.addLayout( buttonsLayout )
    
        connect( fontPushButton, SIGNAL( 'clicked()' ), self, SLOT( 'chooseFont()' ) )
        connect( okPushButton, SIGNAL( 'clicked()' ), self, SLOT( 'accept()' ) )
        connect( cancelPushButton, SIGNAL( 'clicked()' ), self, SLOT( 'reject()' ) )
    
        chartTypeTextLabel.setBuddy( @chartTypeComboBox )
        decimalPlacesTextLabel.setBuddy( @decimalPlacesSpinBox )
    end
    
    
    def chooseFont()
        ok = Qt::Boolean.new
        font = Qt::FontDialog.getFont( ok, @font, self )
        if !ok.nil?
            setFont( font )
        end
    end
    
    
    def setFont( font )
        label = font.family() + " " +
                        font.pointSize().to_s + "pt"
        if  font.bold()
            label += " Bold"
        end
        if  font.italic()
            label += " Italic"
        end
        @fontTextLabel.setText( label )
        @font = font
    end

end
