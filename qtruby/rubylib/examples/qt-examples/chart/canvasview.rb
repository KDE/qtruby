class CanvasView < Qt::CanvasView


    def initialize(canvas, elements, parent = nil, name = "canvas view", f = 0)
        super(canvas, parent, name, f)
        @elements = elements
        @movingItem = nil
    end
    
    def contentsContextMenuEvent( e )
        parent().optionsMenu.exec( Qt::Cursor.pos() )
    end
    
    
    def viewportResizeEvent( e )
        canvas().resize( e.size().width(), e.size().height() )
        parent().drawElements()
    end
    
    
    def contentsMousePressEvent( e )
        list = canvas().collisions( e.pos() )
        list.each do |it|
            if  it.rtti() == CanvasText::CANVAS_TEXT 
                @movingItem = it
                @pos = e.pos()
                return
            end
        end
        @movingItem = nil
    end
    
    
    def contentsMouseMoveEvent( e )
        if  @movingItem 
            offset = e.pos() - @pos
            @movingItem.moveBy( offset.x(), offset.y() )
            @pos = e.pos()
            form = parent()
            form.setChanged( true )
            chartType = form.chartType()
            item = @movingItem
            i = item.index()
    
            @elements[i].setProX( chartType, item.x() / canvas().width() )
            @elements[i].setProY( chartType, item.y() / canvas().height() )
    
            canvas().update()
        end
    end

end


