class ChartForm

    def load( filename )
        file = Qt::File.new( filename )
        if  !file.open( Qt::IO_ReadOnly ) 
            statusBar().message( "Failed to load \'%s\'" % filename, 2000 )
            return
        end
    
        init() # Make sure we have colours
        @filename = filename
        ts = Qt::TextStream.new( file )
        element = Element.new
        errors = 0
        i = 0
        while !ts.eof() 
            ts >> element
            if  element.isValid()
                @elements[i] = element
                i += 1
            else
                errors += 1
            end
            if  i == MAX_ELEMENTS 
                statusBar().message("Read maximum number of elements (%d) discarding others" % i, 2000 )
                break
            end
        end
    
        file.close()
    
        bad = ""
        if errors > 0
            bad = " skipped %d bad record" % errors
            if  errors > 1
                bad += "s"
            end
        end
        statusBar().message( "Read %d values from \'%s\'" % [i, filename], 3000 )
    
        setCaption( "Chart -- %s" % filename )
        updateRecentFiles( filename )
    
        drawElements()
        @changed = false
    end
    
    
    def fileSave()
        if  @filename.isEmpty() 
            fileSaveAs()
            return
        end
    
        file = Qt::File.new( @filename )
        if  !file.open( Qt::IO_WriteOnly ) 
            statusBar().message( "Failed to save \'%s\'" % @filename, 2000 )
            return
        end
        ts = Qt::TextStream.new( file )
        (0...MAX_ELEMENTS).each do |i|
            if  @elements[i].isValid()
                ts << @elements[i]
            end
        end
    
        file.close()
    
        setCaption( "Chart -- %s" % @filename )
        statusBar().message( "Saved \'%s\'" % @filename, 2000 )
        @changed = false
    end
    
    
    def fileSaveAsPixmap()
        filename = Qt::FileDialog.getSaveFileName(nil, "Images (*.png *.xpm *.jpg)",
                                self, "file save as bitmap",
                                "Chart -- File Save As Bitmap" )
        if Qt::Pixmap.grabWidget( @canvasView ).save( filename,
                    filename.sub(/.*\.([^.]*)$/, '\1').upcase() ) 
            statusBar().message( "Wrote \'%s\'" % filename, 2000 )
        else
            statusBar().message( "Failed to write \'%s\'" % filename, 2000 )
        end
    end
    
    def filePrint()
        if  !@printer
            @printer = Qt::Printer.new
        end
        if  @printer.setup() 
            painter = Qt::Painter.new( @printer )
            @canvas.drawArea( Qt::Rect.new( 0, 0, @canvas.width(), @canvas.height() ),
                                painter, false )
            if  !@printer.outputFileName().empty? 
                statusBar().message( "Printed \'%s\'" % @printer.outputFileName(), 2000 )
            end
        end
    end

end

