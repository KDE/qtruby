class MainForm

    CLIP_AS_HEX = 0
    CLIP_AS_NAME = 1
    CLIP_AS_RGB = 2
    COL_NAME = 0
    COL_HEX = 1
    COL_WEB = 2
    WINDOWS_REGISTRY = "/QtExamples"
    APP_KEY = "/ColorTool/"

def init()
    @clipboard = Qt::Application.clipboard()
    if @clipboard.supportsSelection()
        @clipboard.selectionMode = true
    end

    findForm = 0
    loadSettings()
    @filename = nil
    @changed = false
    @table_dirty = true
    @icons_dirty = true
    @colors = {}
    @comments = {}
    clearData( true )
end

def clearData( fillWithDefaults )
    setCaption( "Color Tool" )

    @colors.clear()
    @comments.clear()

    if fillWithDefaults
        @colors["black"] = Qt::black
        @colors["blue"] = Qt::blue
        @colors["cyan"] = Qt::cyan
        @colors["darkblue"] = Qt::darkBlue
        @colors["darkcyan"] = Qt::darkCyan
        @colors["darkgray"] = Qt::darkGray
        @colors["darkgreen"] = Qt::darkGreen
        @colors["darkmagenta"] = Qt::darkMagenta
        @colors["darkred"] = Qt::darkRed
        @colors["darkyellow"] = Qt::darkYellow
        @colors["gray"] = Qt::gray
        @colors["green"] = Qt::green
        @colors["lightgray"] = Qt::lightGray
        @colors["magenta"] = Qt::magenta
        @colors["red"] = Qt::red
        @colors["white"] = Qt::white
        @colors["yellow"] = Qt::yellow
    end

    populate()
end

def populate()
    if @table_dirty
        (0...@colorTable.numRows).each do |r|
            (0...@colorTable.numCols).each do |c|
                @colorTable.clearCell( r, c )
            end
        end

        @colorTable.numRows = @colors.length
        if ! @colors.empty?
            pixmap = Qt::Pixmap.new( 22, 22 )
            row = 0
            @colors.sort.each do |pair|
                key = pair[0]
                color = pair[1]
                pixmap.fill( color )
                @colorTable.setText( row, COL_NAME, key )
                @colorTable.setPixmap( row, COL_NAME, pixmap );
                @colorTable.setText( row, COL_HEX, color.name().upcase() )
                if @show_web
                    item = Qt::CheckTableItem.new( @colorTable, "" )
                    item.checked = webColor?( color )
                    @colorTable.setItem( row, COL_WEB, item )
                end
                row += 1
            end
            @colorTable.setCurrentCell( 0, 0 )
        end
        @colorTable.adjustColumn( COL_NAME )
        @colorTable.adjustColumn( COL_HEX )
        if @show_web
            @colorTable.showColumn( COL_WEB )
            @colorTable.adjustColumn( COL_WEB )
        else
            @colorTable.hideColumn( COL_WEB )
        end
        @table_dirty = FALSE;
    end

    if @icons_dirty
        @colorIconView.clear()

        @colors.each do |key, data|
            Qt::IconViewItem.new( @colorIconView, key, colorSwatch(data) )
        end
        @icons_dirty = false
    end
end

def colorSwatch( color )
    pixmap = Qt::Pixmap.new( 80, 80 )
    pixmap.fill( white )
    painter = Qt::Painter.new
    painter.begin( pixmap )
    painter.pen = NoPen
    painter.brush = color
    painter.drawEllipse( 0, 0, 80, 80 )
    painter.end()
    return pixmap
end

def fileNew()
    if okToClear()
        @filename = nil
        @changed = false
        @table_dirty = true
        @icons_dirty = true
        clearData( false )
    end
end

def fileOpen()
    if ! okToClear()
        return
    end

    filename = Qt::FileDialog.getOpenFileName(
                nil, "Colors (*.txt)", self,
                "file open", "Color Tool -- File Open" )
    if ! filename.nil?
        load( filename )
    else
        statusBar().message( "File Open abandoned", 2000 )
    end
end

def fileSave()
    if @filename.nil?
        fileSaveAs()
        return
    end

    file = Qt::File.new( @filename )
    if file.open( Qt::IO_WriteOnly )
        stream = Qt::TextStream.new( file )
        if ! @comments.empty? 
            stream << @comments + "\n" << "\n"
        end
        
        @colors.each do |key, color|
            stream << "%3d %3d %3d \t\t#{key}" % [color.red, color.green, color.blue] << "\n"
         end
        file.close()
        setCaption( "Color Tool -- #{@filename}" )
        statusBar().message( "Saved #{@colors.length} colors to '#{@filename}'", 3000 )
        @changed = false;
    else
        statusBar().message( "Failed to save '#{@filename}'", 3000 )
    end
end

def fileSaveAs()
    filename = Qt::FileDialog.getSaveFileName(
                nil, "Colors (*.txt)", self,
                "file save as", "Color Tool -- File Save As" )
    if ! filename.nil? 
        ans = 0
        if Qt::File.exists( filename )
           ans = Qt::MessageBox.warning(
                self, "Color Tool -- Overwrite File",
                "Overwrite\n'#{filename}'?" ,
                "&Yes", "&No", nil, 1, 1 )
        end
        if ans == 0 
           @filename = filename
           fileSave()
           return
        end
    end
    statusBar().message( "Saving abandoned", 2000 )
end

def load( filename )
    clearData( false )
    @filename = filename
    regex = Regexp.new( "^\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\S+.*)$" )
    file = Qt::File.new( filename )
    if file.open( Qt::IO_ReadOnly )
        statusBar().message( "Loading '#{filename}'..." )
        stream = Qt::TextStream.new( file )
        while ! stream.eof()
            line = stream.readLine()
            m = regex.match( line )
            if m.nil?
                @comments += line
            else
                @colors[m[4]] = Qt::Color.new(m[1].to_i,m[2].to_i,m[3].to_i )
            end
        end
        file.close()
        @filename = filename
        setCaption( "Color Tool -- #{@filename}" )
        statusBar().message( "Loaded '#{@filename}'", 3000 )
        visible = @colorWidgetStack.visibleWidget()
        @icons_dirty = ! ( @table_dirty = ( visible == @tablePage ) )
        populate()
        @icons_dirty = ! ( @table_dirty = ( visible != @tablePage ) )
        @changed = false
    else
        statusBar().message( "Failed to load '#{@filename}'", 3000 )
    end
end


def okToClear()
    if @changed
        if @filename.nil?
            msg = "Unnamed colors "
        else
            msg = "Colors '#{@filename}'\n"
        end
        msg += "has been changed."
        ans = Qt::MessageBox.information(
            self,
            "Color Tool -- Unsaved Changes",
            msg, "&Save", "Cancel", "&Abandon",
            0, 1 )
        if ans == 0
            fileSave()
        elsif ans == 1
            return false
        end
    end

    return true
end

def closeEvent( e )
    fileExit()
end

def fileExit()
    if okToClear()
        saveSettings()
        Qt::Application.exit( 0 )
    end
end

def editCut()
    visible = @colorWidgetStack.visibleWidget()
    statusBar().message( "Deleting '#{name}'" )

    if visible == @tablePage && @colorTable.numRows() > 0
        row = @colorTable.currentRow()
        name = @colorTable.text( row, 0 )
        @colorTable.removeRow( @colorTable.currentRow() )
        if row < @colorTable.numRows()
            @colorTable.setCurrentCell( row, 0 )
        elsif @colorTable.numRows() > 0
            @colorTable.setCurrentCell( @colorTable.numRows() - 1, 0 )
        end
        @icons_dirty = true
    elsif visible == @iconsPage && @colorIconView.currentItem()
        item = colorIconView.currentItem()
        name = item.text()
        if @colorIconView.count() == 1 
            @colorIconView.clear()
        else 
            current = item.nextItem()
            if ! current
                current = item.prevItem()
            end
            item.dispose
            if current
                @colorIconView.currentItem = current
            end
            @colorIconView.arrangeItemsInGrid()
        end
        @table_dirty = true
    end

    if ! name.nil?
        @colors.delete( name )
        @changed = true
        statusBar().message( "Deleted '#{name}'", 5000 )
    else
        statusBar().message( "Failed to delete '#{name}'", 5000 )
    end
end

def editCopy()
    visible = @colorWidgetStack.visibleWidget()

    if visible == @tablePage && @colorTable.numRows()
        row = @colorTable.currentRow()
        text = @colorTable.text( row, 0 )
    elsif visible == @iconsPage && ! @colorIconView.currentItem().nil?
        item = @colorIconView.currentItem()
        text = item.text()
    end
    if ! text.nil?
        color = @colors[text]
        case @clip_as
        when CLIP_AS_HEX then text = color.name()
        when CLIP_AS_NAME then
        when CLIP_AS_RGB
            text = "#{color.red},#{color.green},#{color.blue}"
        end
        @clipboard.text = text
        statusBar().message( "Copied '" + text + "' to the clipboard" )
    end
end

def editFind()
    if ! @findForm
        @findForm = FindForm.new( self )
        connect( @findForm, SIGNAL( 'lookfor(const QString&)' ),
                 self, SLOT( 'lookfor(const QString&)' ) )
    end
    @findForm.show()
end

def lookfor( text )
    if text.empty?
        return
    end
    ltext = text.downcase()
    visible = colorWidgetStack.visibleWidget()
    found = false

    if visible == @tablePage && @colorTable.numRows() > 0
        row = @colorTable.currentRow()
        (row+1...@colorTable.numRows).each do |i|
            if @colorTable.text( i, 0 ).downcase().include?( ltext )
                @colorTable.setCurrentCell( i, 0 )
                @colorTable.clearSelection()
                @colorTable.selectRow( i )
                found = true
            end
        end
        if ! found
            @colorTable.setCurrentCell( row, 0 )
        end
    elsif visible == @iconsPage
        start = @colorIconView.currentItem()
        item = start.nextItem() unless start.nil?
        while !item.nil?
            if item.text().downcase().include?( ltext ) 
                @colorIconView.currentItem = item
                @colorIconView.ensureItemVisible( item )
                found = true
            end
            item = item.nextItem()
        end
        if ! found && !start.nil?
            @colorIconView.currentItem = start
        end
    end
    if ! found
        statusBar().message( "Could not find '#{text}' after here" )
        @findForm.notfound()
    end
end



def helpIndex()
end

def helpContents()
end

def helpAbout()
end


def changedTableColor( row, i )
    changedColor( @colorTable.text( row, COL_NAME ) )
end

def changedIconColor( item )
    changedColor( item.text() )
end

def changedColor( name )
    color = @colors[name]
    r = color.red()
    g = color.green()
    b = color.blue()
    statusBar().message( "%s \"%s\" (%d,%d,%d) %s {%.3f %.3f %.3f}" % 
                         [name, color.name.upcase, 
                         r, g, b, webColor?( color ) ? ' web' : '', 
                         r / 255.0, g / 255.0, b / 255.0] )
end

def changeView(action)
    if action == @viewTableAction
        @colorWidgetStack.raiseWidget( @tablePage )
    else
        @colorWidgetStack.raiseWidget( @iconsPage )
    end
end

def webColor?( color )
    r = color.red()
    g = color.green()
    b = color.blue()

    return ( ( r ==   0 || r ==  51 || r == 102 ||
           r == 153 || r == 204 || r == 255 ) &&
         ( g ==   0 || g ==  51 || g == 102 ||
           g == 153 || g == 204 || g == 255 ) &&
         ( b ==   0 || b ==  51 || b == 102 ||
           b == 153 || b == 204 || b == 255 ) )
end


def editAdd()
    color = Qt::white
    if ! @colors.empty?
        visible = @colorWidgetStack.visibleWidget()
        if visible == @tablePage
            color = Qt::Color.new(@colorTable.text( @colorTable.currentRow(),
                          @colorTable.currentColumn() ))
        else
            color = Qt::Color.new(@colorIconView.currentItem().text())
        end
    end
    color = Qt::ColorDialog.getColor( color, self )
    if color.valid?
        pixmap = Qt::Pixmap.new( 80, 10 )
        pixmap.fill( color )
        colorForm = ColorNameForm.new( self, "color", true )
        colorForm.setColors( @colors )
        colorForm.colorLabel.setPixmap( pixmap )
        if colorForm.exec()
            name = colorForm.colorLineEdit.text()
            @colors[name] = color
            pixmap = Qt::Pixmap.new( 22, 22 )
            pixmap.fill( color )
            row = @colorTable.currentRow()
            @colorTable.insertRows( row, 1 )
            @colorTable.setText( row, COL_NAME, name )
            @colorTable.setPixmap( row, COL_NAME, pixmap )
            @colorTable.setText( row, COL_HEX, color.name().upcase() )
            if @show_web
                item = Qt::CheckTableItem.new( @colorTable, "" )
                item.checked = webColor?( color )
                @colorTable.setItem( row, COL_WEB, item )
            end
            @colorTable.setCurrentCell( row, 0 )

            Qt::IconViewItem.new( @colorIconView, name,
                                  colorSwatch( color ) )
            @changed = true
        end
    end
end

def editOptions()
    options = OptionsForm.new( self, "options", true )
    case @clip_as
    when CLIP_AS_HEX
        options.hexRadioButton.checked = true
    when CLIP_AS_NAME
        options.nameRadioButton.checked = true 
    when CLIP_AS_RGB
        options.rgbRadioButton.checked = true
    end
    options.webCheckBox.checked = @show_web

    if options.exec()
        if options.hexRadioButton.checked?
            @clip_as = CLIP_AS_HEX
        elsif options.nameRadioButton.checked?
            @clip_as = CLIP_AS_NAME
        elsif options.rgbRadioButton.checked?
            @clip_as = CLIP_AS_RGB
        end
        @table_dirty = @show_web != options.webCheckBox.checked?
        @show_web = options.webCheckBox.checked?
        populate()
    end
end

def loadSettings()
    settings = Qt::Settings.new
    settings.insertSearchPath( Qt::Settings::Windows, WINDOWS_REGISTRY )
    windowWidth = settings.readNumEntry( APP_KEY + "WindowWidth", 550 )
    windowHeight = settings.readNumEntry( APP_KEY + "WindowHeight", 500 )
    windowX = settings.readNumEntry( APP_KEY + "WindowX", 0 )
    windowY = settings.readNumEntry( APP_KEY + "WindowY", 0 )
    @clip_as = settings.readNumEntry( APP_KEY + "ClipAs", CLIP_AS_HEX )
    @show_web = settings.readBoolEntry( APP_KEY + "ShowWeb", true )
    if ! settings.readBoolEntry( APP_KEY + "View", true )
        @colorWidgetStack.raiseWidget( @iconsPage )
        @viewIconsAction.on = true
    end

    resize( windowWidth, windowHeight )
    move( windowX, windowY )
end

def saveSettings()
    settings = Qt::Settings.new
    settings.insertSearchPath( Qt::Settings::Windows, WINDOWS_REGISTRY )
    settings.writeEntry( APP_KEY + "WindowWidth", width() )
    settings.writeEntry( APP_KEY + "WindowHeight", height() )
    settings.writeEntry( APP_KEY + "WindowX", x() )
    settings.writeEntry( APP_KEY + "WindowY", y() )
    settings.writeEntry( APP_KEY + "ClipAs", @clip_as )
    settings.writeEntry( APP_KEY + "ShowWeb", @show_web )
    settings.writeEntry( APP_KEY + "View", @colorWidgetStack.visibleWidget() == @tablePage )
end


def aboutToShow()
    populate()
end

end
