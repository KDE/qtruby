class SetDataForm < Qt::Dialog

    slots 'setColor()',
        'setColor( int, int )',
        'currentChanged( int, int )',
        'valueChanged( int, int )',
        'accept()'

    MAX_PATTERNS = 14


    def initialize( elements, decimalPlaces,
                            parent = nil,  name = "set data form",
                            modal = true, f = 0 )
        super( parent, name, modal, f )
    
        @elements = elements
        @decimalPlaces = decimalPlaces
    
        setCaption( "Chart -- Set Data" )
        resize( 540, 440 )
    
        @tableButtonBox = Qt::VBoxLayout.new( self, 11, 6, "@table button box layout" )
    
        @table = Qt::Table.new( self, "data @table" )
        @table.setNumCols( 5 )
        @table.setNumRows( ChartForm::MAX_ELEMENTS )
        @table.setColumnReadOnly( 1, true )
        @table.setColumnReadOnly( 2, true )
        @table.setColumnReadOnly( 4, true )
        @table.setColumnWidth( 0, 80 )
        @table.setColumnWidth( 1, 60 ) # Columns 1 and 4 must be equal
        @table.setColumnWidth( 2, 60 )
        @table.setColumnWidth( 3, 200 )
        @table.setColumnWidth( 4, 60 )
        th = @table.horizontalHeader()
        th.setLabel( 0, "Value" )
        th.setLabel( 1, "Color" )
        th.setLabel( 2, "Pattern" )
        th.setLabel( 3, "Label" )
        th.setLabel( 4, "Color" )
        @tableButtonBox.addWidget( @table )
    
        @buttonBox = Qt::HBoxLayout.new( nil, 0, 6, "button box layout" )
    
        @colorPushButton = Qt::PushButton.new( self, "color button" )
        @colorPushButton.setText( "&Color..." )
        @colorPushButton .setEnabled( false )
        @buttonBox.addWidget( @colorPushButton )
    
        spacer = Qt::SpacerItem.new( 0, 0, Qt::SizePolicy::Expanding,
                                                    Qt::SizePolicy::Minimum )
        @buttonBox.addItem( spacer )
    
        okPushButton = Qt::PushButton.new( self, "ok button" )
        okPushButton.setText( "OK" )
        okPushButton.setDefault( true )
        @buttonBox.addWidget( okPushButton )
    
        cancelPushButton = Qt::PushButton.new( self, "cancel button" )
        cancelPushButton.setText( "Cancel" )
        cancelPushButton.setAccel( Qt::KeySequence.new(Key_Escape) )
        @buttonBox.addWidget( cancelPushButton )
    
        @tableButtonBox.addLayout( @buttonBox )
    
        connect( @table, SIGNAL( 'clicked(int,int,int,const QPoint&)' ),
                self, SLOT( 'setColor(int,int)' ) )
        connect( @table, SIGNAL( 'currentChanged(int,int)' ),
                self, SLOT( 'currentChanged(int,int)' ) )
        connect( @table, SIGNAL( 'valueChanged(int,int)' ),
                self, SLOT( 'valueChanged(int,int)' ) )
        connect( @colorPushButton, SIGNAL( 'clicked()' ), self, SLOT( 'setColor()' ) )
        connect( okPushButton, SIGNAL( 'clicked()' ), self, SLOT( 'accept()' ) )
        connect( cancelPushButton, SIGNAL( 'clicked()' ), self, SLOT( 'reject()' ) )
    
        patterns = Array.new(MAX_PATTERNS)
        patterns[0]  = Qt::Pixmap.new( "images/pattern01.xpm" )
        patterns[1]  = Qt::Pixmap.new( "images/pattern02.xpm" )
        patterns[2]  = Qt::Pixmap.new( "images/pattern03.xpm" )
        patterns[3]  = Qt::Pixmap.new( "images/pattern04.xpm" )
        patterns[4]  = Qt::Pixmap.new( "images/pattern05.xpm" )
        patterns[5]  = Qt::Pixmap.new( "images/pattern06.xpm" )
        patterns[6]  = Qt::Pixmap.new( "images/pattern07.xpm" )
        patterns[7]  = Qt::Pixmap.new( "images/pattern08.xpm" )
        patterns[8]  = Qt::Pixmap.new( "images/pattern09.xpm" )
        patterns[9]  = Qt::Pixmap.new( "images/pattern10.xpm" )
        patterns[10] = Qt::Pixmap.new( "images/pattern11.xpm" )
        patterns[11] = Qt::Pixmap.new( "images/pattern12.xpm" )
        patterns[12] = Qt::Pixmap.new( "images/pattern13.xpm" )
        patterns[13] = Qt::Pixmap.new( "images/pattern14.xpm" )
    
        rect = @table.cellRect( 0, 1 )
        pix = Qt::Pixmap.new( rect.width(), rect.height() )
    
        for i in 0...ChartForm::MAX_ELEMENTS
            element = @elements[i]
    
            if element.isValid()
                @table.setText(i, 0, "%.#{@decimalPlaces}f" % element.value() )
            end
    
            color = element.valueColor()
            pix.fill( color )
            @table.setPixmap( i, 1, pix )
            @table.setText( i, 1, color.name() )
    
            combobox = Qt::ComboBox.new
            for j in 0...MAX_PATTERNS
                combobox.insertItem( patterns[j] )
            end
            combobox.setCurrentItem( element.valuePattern() - 1 )
            @table.setCellWidget( i, 2, combobox )
    
            @table.setText( i, 3, element.label() )
    
            color = element.labelColor()
            pix.fill( color )
            @table.setPixmap( i, 4, pix )
            @table.setText( i, 4, color.name() )
        end
    
    end
    
    
    def currentChanged( i, col )
        @colorPushButton.setEnabled( col == 1 || col == 4 )
    end
    
    
    def valueChanged( row, col )
        if col == 0 
            d = @table.text( row, col ).to_f
            if d && d > EPSILON
                @table.setText( row, col, "%.#{@decimalPlaces}f" % d )
            elsif ! @table.text( row, col ).empty?
                @table.setText( row, col, @table.text( row, col ) + "?" )
            end
        end
    end
    
    
    def setColor()
        setColor( @table.currentRow(), @table.currentColumn() )
        @table.setFocus()
    end
    
    
    def setColor( row, col )
        if  !( col == 1 || col == 4 )
            return
        end
    
        color = Qt::ColorDialog.getColor(
                            Qt::Color.new( @table.text( row, col ) ),
                            self, "color dialog" )
        if  color.isValid() 
            pix = @table.pixmap( row, col )
            pix.fill( color )
            @table.setPixmap( row, col, pix )
            @table.setText( row, col, color.name() )
        end
    end
    
    
    def accept()
        for i in 0...ChartForm::MAX_ELEMENTS
            element = @elements[i]
            d = @table.text( i, 0 ).to_f
            if d
                element.value = d 
            else
                element.value = Element::INVALID
            end
            element.valueColor = Qt::Color.new( @table.text( i, 1 ) ) 
            element.valuePattern = (@table.cellWidget( i, 2 )).currentItem() + 1
            element.label = @table.text( i, 3 )
            element.labelColor = Qt::Color.new( @table.text( i, 4 ) )
        end
    
        super
    end

end
