class ChartForm < Qt::MainWindow
    
    slots 'fileNew()',
        'fileOpen()',
        'fileOpenRecent( int )',
        'fileSave()',
        'fileSaveAs()',
        'fileSaveAsPixmap()',
        'filePrint()',
        'fileQuit()',
        'optionsSetData()',
        'updateChartType( QAction * )',
        'optionsSetFont()',
        'optionsSetOptions()',
        'helpHelp()',
        'helpAbout()',
        'helpAboutQt()',
        'saveOptions()'

    MAX_ELEMENTS = 100
    MAX_RECENTFILES = 9 # Must not exceed 9
    
    PIE = 0
    VERTICAL_BAR = 1
    HORIZONTAL_BAR = 2
    
    NO = 0
    YES = 1
    AS_PERCENTAGE = 2
    
    WINDOWS_REGISTRY = "/Trolltech/QtExamples"
    APP_KEY = "/Chart/"

    def initialize( filename )
        super( nil, nil, WDestructiveClose )
        @filename = filename
        setIcon( Qt::Pixmap.new( "images/options_piechart.xpm" ) )
    
        fileNewAction = Qt::Action.new(
            "New Chart", Qt::IconSet.new(Qt::Pixmap.new( "images/file_new.xpm" )),
            "&New", Qt::KeySequence.new(CTRL+Key_N), self, "new" )
        connect( fileNewAction, SIGNAL( 'activated()' ), self, SLOT( 'fileNew()' ) )
    
        fileOpenAction = Qt::Action.new(
            "Open Chart", Qt::IconSet.new(Qt::Pixmap.new( "images/file_open.xpm" )),
            "&Open...", Qt::KeySequence.new(CTRL+Key_O), self, "open" )
        connect( fileOpenAction, SIGNAL( 'activated()' ), self, SLOT( 'fileOpen()' ) )
    
        fileSaveAction = Qt::Action.new(
            "Save Chart", Qt::IconSet.new(Qt::Pixmap.new( "images/file_save.xpm" )),
            "&Save", Qt::KeySequence.new(CTRL+Key_S), self, "save" )
        connect( fileSaveAction, SIGNAL( 'activated()' ), self, SLOT( 'fileSave()' ) )
    
        fileSaveAsAction = Qt::Action.new(
            "Save Chart As", Qt::IconSet.new(Qt::Pixmap.new( "images/file_save.xpm" )),
            "Save &As...", Qt::KeySequence.new(0), self, "save as" )
        connect( fileSaveAsAction, SIGNAL( 'activated()' ),
            self, SLOT( 'fileSaveAs()' ) )
    
        fileSaveAsPixmapAction = Qt::Action.new(
            "Save Chart As Bitmap", Qt::IconSet.new(Qt::Pixmap.new( "images/file_save.xpm" )),
            "Save As &Bitmap...", Qt::KeySequence.new(CTRL+Key_B), self, "save as bitmap" )
        connect( fileSaveAsPixmapAction, SIGNAL( 'activated()' ),
            self, SLOT( 'fileSaveAsPixmap()' ) )
    
        filePrintAction = Qt::Action.new(
            "Print Chart", Qt::IconSet.new(Qt::Pixmap.new( "images/file_print.xpm" )),
            "&Print Chart...", Qt::KeySequence.new(CTRL+Key_P), self, "print chart" )
        connect( filePrintAction, SIGNAL( 'activated()' ),
            self, SLOT( 'filePrint()' ) )
    
        optionsSetDataAction = Qt::Action.new(
            "Set Data", Qt::IconSet.new(Qt::Pixmap.new( "images/options_setdata.xpm" )),
            "Set &Data...", Qt::KeySequence.new(CTRL+Key_D), self, "set data" )
        connect( optionsSetDataAction, SIGNAL( 'activated()' ),
            self, SLOT( 'optionsSetData()' ) )
    
    
        chartGroup = Qt::ActionGroup.new( self ) # Connected later
        chartGroup.setExclusive( true )
    
        @optionsPieChartAction = Qt::Action.new(
            "Pie Chart", Qt::IconSet.new(Qt::Pixmap.new( "images/options_piechart.xpm" )),
            "&Pie Chart", Qt::KeySequence.new(CTRL+Key_I), chartGroup, "pie chart" )
        @optionsPieChartAction.setToggleAction( true )
    
        @optionsHorizontalBarChartAction = Qt::Action.new(
            "Horizontal Bar Chart", Qt::IconSet.new(Qt::Pixmap.new( "images/options_horizontalbarchart.xpm" )),
            "&Horizontal Bar Chart", Qt::KeySequence.new(CTRL+Key_H), chartGroup,
            "horizontal bar chart" )
        @optionsHorizontalBarChartAction.setToggleAction( true )
    
        @optionsVerticalBarChartAction = Qt::Action.new(
            "Vertical Bar Chart", Qt::IconSet.new(Qt::Pixmap.new( "images/options_verticalbarchart.xpm" )),
            "&Vertical Bar Chart", Qt::KeySequence.new(CTRL+Key_V), chartGroup, "Vertical bar chart" )
        @optionsVerticalBarChartAction.setToggleAction( true )
    
    
        optionsSetFontAction = Qt::Action.new(
            "Set Font", Qt::IconSet.new(Qt::Pixmap.new( "images/options_setfont.xpm" )),
            "Set &Font...", Qt::KeySequence.new(CTRL+Key_F), self, "set font" )
        connect( optionsSetFontAction, SIGNAL( 'activated()' ),
            self, SLOT( 'optionsSetFont()' ) )
    
        optionsSetOptionsAction = Qt::Action.new(
            "Set Options", Qt::IconSet.new(Qt::Pixmap.new( "images/options_setoptions.xpm" )),
            "Set &Options...", Qt::KeySequence.new(0), self, "set options" )
        connect( optionsSetOptionsAction, SIGNAL( 'activated()' ),
            self, SLOT( 'optionsSetOptions()' ) )
    
        fileQuitAction = Qt::Action.new( "Quit", "&Quit", Qt::KeySequence.new(CTRL+Key_Q), self, "quit" )
        connect( fileQuitAction, SIGNAL( 'activated()' ), self, SLOT( 'fileQuit()' ) )
    
    
        fileTools = Qt::ToolBar.new( self, "file operations" )
        fileTools.setLabel( "File Operations" )
        fileNewAction.addTo( fileTools )
        fileOpenAction.addTo( fileTools )
        fileSaveAction.addTo( fileTools )
        fileTools.addSeparator()
        filePrintAction.addTo( fileTools )
    
        optionsTools = Qt::ToolBar.new( self, "options operations" )
        optionsTools.setLabel( "Options Operations" )
        optionsSetDataAction.addTo( optionsTools )
        optionsTools.addSeparator()
        @optionsPieChartAction.addTo( optionsTools )
        @optionsHorizontalBarChartAction.addTo( optionsTools )
        @optionsVerticalBarChartAction.addTo( optionsTools )
        optionsTools.addSeparator()
        optionsSetFontAction.addTo( optionsTools )
        optionsTools.addSeparator()
        optionsSetOptionsAction.addTo( optionsTools )
    
        @fileMenu = Qt::PopupMenu.new( self )
        menuBar().insertItem( "&File", @fileMenu )
        fileNewAction.addTo( @fileMenu )
        fileOpenAction.addTo( @fileMenu )
        fileSaveAction.addTo( @fileMenu )
        fileSaveAsAction.addTo( @fileMenu )
        @fileMenu.insertSeparator()
        fileSaveAsPixmapAction.addTo( @fileMenu )
        @fileMenu.insertSeparator()
        filePrintAction.addTo( @fileMenu )
        @fileMenu.insertSeparator()
        fileQuitAction.addTo( @fileMenu )
    
        optionsMenu = Qt::PopupMenu.new( self )
        menuBar().insertItem( "&Options", optionsMenu )
        optionsSetDataAction.addTo( optionsMenu )
        optionsMenu.insertSeparator()
        @optionsPieChartAction.addTo( optionsMenu )
        @optionsHorizontalBarChartAction.addTo( optionsMenu )
        @optionsVerticalBarChartAction.addTo( optionsMenu )
        optionsMenu.insertSeparator()
        optionsSetFontAction.addTo( optionsMenu )
        optionsMenu.insertSeparator()
        optionsSetOptionsAction.addTo( optionsMenu )
    
        menuBar().insertSeparator()
    
        helpMenu = Qt::PopupMenu.new( self )
        menuBar().insertItem( "&Help", helpMenu )
        helpMenu.insertItem( "&Help", self, SLOT('helpHelp()'), Qt::KeySequence.new(Key_F1) )
        helpMenu.insertItem( "&About", self, SLOT('helpAbout()') )
        helpMenu.insertItem( "About &Qt", self, SLOT('helpAboutQt()') )
    
    
        @printer = nil
		@elements = Array.new(MAX_ELEMENTS)
    
        settings = Qt::Settings.new
        settings.insertSearchPath( Qt::Settings::Windows, WINDOWS_REGISTRY )
        windowWidth = settings.readNumEntry( APP_KEY + "WindowWidth", 460 )
        windowHeight = settings.readNumEntry( APP_KEY + "WindowHeight", 530 )
        windowX = settings.readNumEntry( APP_KEY + "WindowX", -1 )
        windowY = settings.readNumEntry( APP_KEY + "WindowY", -1 )
        setChartType( settings.readNumEntry( APP_KEY + "ChartType", PIE )  )
        @addValues = settings.readNumEntry( APP_KEY + "AddValues", NO )
        @decimalPlaces = settings.readNumEntry( APP_KEY + "Decimals", 2 )
        @font = Qt::Font.new( "Helvetica", 18, Qt::Font::Bold )
        @font.fromString(
            settings.readEntry( APP_KEY + "Font", @font.toString() ) )
        @recentFiles = []
        (0...MAX_RECENTFILES).each do |i|
            filename = settings.readEntry( APP_KEY + "File" + ( i + 1 ).to_s )
            if !filename.nil?
                @recentFiles.push( filename )
            end
        end
        if @recentFiles.length() > 0
            updateRecentFilesMenu()
        end
    
    
        # Connect *after* we've set the chart type on so we don't call
        # drawElements() prematurely.
        connect( chartGroup, SIGNAL( 'selected(QAction*)' ),
            self, SLOT( 'updateChartType(QAction*)' ) )
    
        resize( windowWidth, windowHeight )
        if windowX != -1 || windowY != -1
            move( windowX, windowY )
        end
    
        @canvas = Qt::Canvas.new( self )
        @canvas.resize( width(), height() )
        @canvasView = CanvasView.new( @canvas, @elements, self )
        setCentralWidget( @canvasView )
        @canvasView.show()
    
        if ! @filename.nil?
            load( @filename )
        else
            init()
            @elements[0].set( 20, red,    14, "Red" )
            @elements[1].set( 70, cyan,    2, "Cyan",   darkGreen )
            @elements[2].set( 35, blue,   11, "Blue" )
            @elements[3].set( 55, yellow,  1, "Yellow", darkBlue )
            @elements[4].set( 80, magenta, 1, "Magenta" )
            drawElements()
        end
    
        statusBar().message( "Ready", 2000 )
    end
    
    
    
    def init()
        setCaption( "Chart" )
        @filename = nil
        @changed = false
    
        @elements[0]  = Element.new( Element::INVALID, red )
        @elements[1]  = Element.new( Element::INVALID, cyan )
        @elements[2]  = Element.new( Element::INVALID, blue )
        @elements[3]  = Element.new( Element::INVALID, yellow )
        @elements[4]  = Element.new( Element::INVALID, green )
        @elements[5]  = Element.new( Element::INVALID, magenta )
        @elements[6]  = Element.new( Element::INVALID, darkYellow )
        @elements[7]  = Element.new( Element::INVALID, darkRed )
        @elements[8]  = Element.new( Element::INVALID, darkCyan )
        @elements[9]  = Element.new( Element::INVALID, darkGreen )
        @elements[10] = Element.new( Element::INVALID, darkMagenta )
        @elements[11] = Element.new( Element::INVALID, darkBlue )
        (12...MAX_ELEMENTS).each do |i|
            x = (i.to_f / MAX_ELEMENTS) * 360
            y = ((x * 256) % 105) + 151
            z = ((i * 17) % 105) + 151;
            @elements[i] = Element.new( Element::INVALID, Qt::Color.new( x, y, z, Qt::Color::Hsv ) )
        end
    end
    
    def closeEvent( e )
        fileQuit()
    end
    
    
    def fileNew()
        if okToClear()
            init()
            drawElements()
        end
    end
    
    
    def fileOpen()
        if !okToClear()
            return
        end
    
        filename = Qt::FileDialog.getOpenFileName(
                    nil, "Charts (*.cht)", self,
                    "file open", "Chart -- File Open" )
        if !filename.nil?
            load( filename )
        else
            statusBar().message( "File Open abandoned", 2000 )
        end
    end
    
    
    def fileSaveAs()
        filename = Qt::FileDialog.getSaveFileName(
                    nil, "Charts (*.cht)", self,
                    "file save as", "Chart -- File Save As" )
        if !filename.nil?
            answer = 0
            if Qt::File.exists( filename )
                answer = Qt::MessageBox.warning(
                        self, "Chart -- Overwrite File",
                        "Overwrite\n\'#{filename}\'?", 
                        "&Yes", "&No", nil, 1, 1 )
            end
            if answer == 0 
                @filename = filename
                updateRecentFiles( filename )
                fileSave()
                return
            end
        end
        statusBar().message( "Saving abandoned", 2000 )
    end
    
    
    def fileOpenRecent( index )
        if !okToClear()
            return
        end
    
        load( @recentFiles[index] )
    end
    
    
    def updateRecentFiles( filename )
        if @recentFiles.include?( filename )
            return
        end
    
        @recentFiles.push( filename )
        if @recentFiles.length() > MAX_RECENTFILES
            @recentFiles.shift()
        end
    
        updateRecentFilesMenu()
    end
    
    
    def updateRecentFilesMenu()
        (0...MAX_RECENTFILES).each do |i|
            if @fileMenu.findItem( i )
                @fileMenu.removeItem( i )
            end
            if i < @recentFiles.length()
                @fileMenu.insertItem( "&%d %s" % [i + 1, @recentFiles[i]],
                    self, SLOT( 'fileOpenRecent(int)' ),
                    Qt::KeySequence.new(0), i )
            end
        end
    end
    
    
    def fileQuit()
        if okToClear()
            saveOptions()
            $qApp.exit( 0 )
        end
    end
    
    
    def okToClear()
        if @changed
            if @filename.nil?
                msg = "Unnamed chart "
            else
                msg = "Chart '#{@filename}'\n"
            end
            msg += "has been changed."
        
            x = Qt::MessageBox.information( self, "Chart -- Unsaved Changes",
                            msg, "&Save", "Cancel", "&Abandon",
                            0, 1 )
            case x 
                when 0 # Save
                    fileSave()
                when 1 # Cancel
                when 2 # Abandon
                else
                    return false
            end
        end
        return true
    end
    
    
    def saveOptions()
        settings = Qt::Settings.new
        settings.insertSearchPath( Qt::Settings::Windows, WINDOWS_REGISTRY )
        settings.writeEntry( APP_KEY + "WindowWidth", width() )
        settings.writeEntry( APP_KEY + "WindowHeight", height() )
        settings.writeEntry( APP_KEY + "WindowX", x() )
        settings.writeEntry( APP_KEY + "WindowY", y() )
        settings.writeEntry( APP_KEY + "ChartType", @chartType )
        settings.writeEntry( APP_KEY + "AddValues", @addValues )
        settings.writeEntry( APP_KEY + "Decimals", @decimalPlaces )
        settings.writeEntry( APP_KEY + "Font", @font.toString() )
        (0...@recentFiles.length).each do |i|
            settings.writeEntry( APP_KEY + "File" + ( i + 1 ).to_s,
                        @recentFiles[i] )
        end
    end
    
    
    def optionsSetData()
        setDataForm = SetDataForm.new( @elements, @decimalPlaces, self )
        if setDataForm.exec()
            @changed = true
            drawElements()
        end
        setDataForm.dispose
    end
    
    
    def setChartType( chartType )
        @chartType = chartType;
        case @chartType
        when PIE
            @optionsPieChartAction.setOn( true )
        when VERTICAL_BAR:
            @optionsVerticalBarChartAction.setOn( true )
        when HORIZONTAL_BAR:
            @optionsHorizontalBarChartAction.setOn( true )
        end
    end
    
    
    def updateChartType( action )
        if action == @optionsPieChartAction
            @chartType = PIE
        elsif action == @optionsHorizontalBarChartAction
            @chartType = HORIZONTAL_BAR
        elsif action == @optionsVerticalBarChartAction
            @chartType = VERTICAL_BAR
        end
    
        drawElements()
    end
    
    
    def optionsSetFont()
        ok = Qt::Boolean.new
        font = Qt::FontDialog.getFont( ok, @font, self )
        if !ok.nil?
            @font = font
            drawElements()
        end
    end
    
    
    def optionsSetOptions()
        optionsForm = OptionsForm.new( self )
        optionsForm.chartTypeComboBox.setCurrentItem( @chartType )
        optionsForm.setFont( @font )
        case @addValues
        when NO
            optionsForm.noRadioButton.setChecked( true )
        when YES
            optionsForm.yesRadioButton.setChecked( true )
        when AS_PERCENTAGE
            optionsForm.asPercentageRadioButton.setChecked( true )
        end
        optionsForm.decimalPlacesSpinBox.setValue( @decimalPlaces )
        if optionsForm.exec()
            setChartType( optionsForm.chartTypeComboBox.currentItem() )
            @font = optionsForm.font()
            if optionsForm.noRadioButton.isChecked()
                @addValues = NO
            elsif optionsForm.yesRadioButton.isChecked()
                @addValues = YES
            elsif optionsForm.asPercentageRadioButton.isChecked()
                @addValues = AS_PERCENTAGE
            end
            @decimalPlaces = optionsForm.decimalPlacesSpinBox.value()
            drawElements()
        end
    end
    
    
    def helpHelp()
        statusBar().message( "Help is not implemented yet", 2000 )
    end
    
    
    def helpAbout()
        Qt::MessageBox.about( self, "Chart -- About",
                "<center><h1><font color=blue>Chart<font></h1></center>" +
                "<p>Chart your data with <i>chart</i>.</p>" )
    end
    
    
    def helpAboutQt()
        Qt::MessageBox.aboutQt( self, "Chart -- About Qt" )
    end

end

