class ChartForm

    def drawElements()
        list = @canvas.allItems()
        list.each do |it|
            it.dispose
        end
    
            # 360 * 16 for pies Qt works with 16ths of degrees
        scaleFactor = @chartType == PIE ? 5760 :
                            @chartType == VERTICAL_BAR ? @canvas.height() :
                                @canvas.width()
        biggest = 0.0
        count = 0
        total = 0.0
        scales = Array.new(MAX_ELEMENTS)
    
        for i in 0...MAX_ELEMENTS
            if  @elements[i].isValid() 
                value = @elements[i].value()
                count += 1
                total += value
                if  value > biggest
                    biggest = value
                end
                scales[i] = @elements[i].value() * scaleFactor
            end
        end
    
        if count > 0
                # 2nd loop because of total and biggest
            for i in 0...MAX_ELEMENTS
                if  @elements[i].isValid()
                    if  @chartType == PIE
                        scales[i] = (@elements[i].value() * scaleFactor) / total
                    else
                        scales[i] = (@elements[i].value() * scaleFactor) / biggest
                    end
                end
            end
    
            case @chartType 
                when PIE
                    drawPieChart( scales, total, count )
                when VERTICAL_BAR:
                    drawVerticalBarChart( scales, total, count )
                when HORIZONTAL_BAR:
                    drawHorizontalBarChart( scales, total, count )
            end
        end
    
        @canvas.update()
    end
    
    
    def drawPieChart( scales, total, i )
        width = @canvas.width().to_f
        height = @canvas.height().to_f
        size = width > height ? height : width
        x = width / 2
        y = height / 2
        angle = 0
    
        for i in 0...MAX_ELEMENTS
            if  @elements[i].isValid() 
                extent = scales[i]
                arc = Qt::CanvasEllipse.new( size, size, angle, extent, @canvas )
                arc.setX( x )
                arc.setY( y )
                arc.setZ( 0 )
                arc.setBrush( Qt::Brush.new( @elements[i].valueColor(),
                                    @elements[i].valuePattern() ) )
                arc.show()
                angle += extent
                label = @elements[i].label()
                if  !label.empty? || @addValues != NO 
                    label = valueLabel( label, @elements[i].value(), total )
                    text = CanvasText.new( i, label, @font, @canvas )
                    proX = @elements[i].proX( PIE ).to_f
                    proY = @elements[i].proY( PIE ).to_f
                    if  proX < 0 || proY < 0 
                        # Find the centre of the pie segment
                        rect = arc.boundingRect()
                        proX = ( rect.width() / 2 ) + rect.x()
                        proY = ( rect.height() / 2 ) + rect.y()
                        # Centre text over the centre of the pie segment
                        rect = text.boundingRect()
                        proX -= ( rect.width() / 2 )
                        proY -= ( rect.height() / 2 )
                        # Make proportional
                        proX /= width
                        proY /= height
                    end
                    text.setColor( @elements[i].labelColor() )
                    text.setX( proX * width )
                    text.setY( proY * height )
                    text.setZ( 1 )
                    text.show()
                    @elements[i].setProX( PIE, proX )
                    @elements[i].setProY( PIE, proY )
                end
            end
        end
    end
    
    
    def drawVerticalBarChart(scales, total, count )
        width = @canvas.width().to_f
        height = @canvas.height().to_f
        prowidth = width / count
        x = 0
        pen = Qt::Pen.new
        pen.style = NoPen
    
        for i in 0...MAX_ELEMENTS
            if  @elements[i].isValid() 
                extent = scales[i]
                y = height - extent
                rect = Qt::CanvasRectangle.new(x, y, prowidth, extent, @canvas )
                rect.setBrush( Qt::Brush.new( @elements[i].valueColor(),
                                        @elements[i].valuePattern() ) )
                rect.setPen( pen )
                rect.setZ( 0 )
                rect.show()
                label = @elements[i].label()
                if  !label.empty? || @addValues != NO 
                    proX = @elements[i].proX( VERTICAL_BAR ).to_f
                    proY = @elements[i].proY( VERTICAL_BAR ).to_f
                    if  proX < 0 || proY < 0 
                        proX = x / width
                        proY = y / height
                    end
                    label = valueLabel( label, @elements[i].value(), total )
                    text = CanvasText.new( i, label, @font, @canvas )
                    text.setColor( @elements[i].labelColor() )
                    text.setX( proX * width )
                    text.setY( proY * height )
                    text.setZ( 1 )
                    text.show()
                    @elements[i].setProX( VERTICAL_BAR, proX )
                    @elements[i].setProY( VERTICAL_BAR, proY )
                end
                x += prowidth
            end
        end
    end
    
    
    def drawHorizontalBarChart(scales, total, count )
        width = @canvas.width().to_f
        height = @canvas.height().to_f
        proheight = height / count
        y = 0
        pen = Qt::Pen.new
        pen.style = NoPen
    
        for i in 0...MAX_ELEMENTS
            if  @elements[i].isValid() 
                extent = scales[i]
                rect = Qt::CanvasRectangle.new(0, y, extent, proheight, @canvas )
                rect.setBrush( Qt::Brush.new( @elements[i].valueColor(),
                                        @elements[i].valuePattern() ) )
                rect.setPen( pen )
                rect.setZ( 0 )
                rect.show()
                label = @elements[i].label()
                if  !label.empty? || @addValues != NO 
                    proX = @elements[i].proX( HORIZONTAL_BAR ).to_f
                    proY = @elements[i].proY( HORIZONTAL_BAR ).to_f
                    if  proX < 0 || proY < 0 
                        proX = 0
                        proY = y / height
                    end
                    label = valueLabel( label, @elements[i].value(), total )
                    text = CanvasText.new( i, label, @font, @canvas )
                    text.setColor( @elements[i].labelColor() )
                    text.setX( proX * width )
                    text.setY( proY * height )
                    text.setZ( 1 )
                    text.show()
                    @elements[i].setProX( HORIZONTAL_BAR, proX )
                    @elements[i].setProY( HORIZONTAL_BAR, proY )
                end
                y += proheight
            end
        end
    end
    
    
    def valueLabel(label, value, total )
        if  @addValues == NO
            return label
        end
    
        newLabel = label
        if  !label.empty?
            if  @chartType == VERTICAL_BAR
                newLabel += "\n"
            else
                newLabel += ' '
            end
        end
        if  @addValues == YES
            newLabel += "%.#{@decimalPlaces}f" % value
        elsif  @addValues == AS_PERCENTAGE
            newLabel += "%.#{@decimalPlaces}f%s" % [(value / total) * 100, '%']
        end
        return newLabel
    end

end

