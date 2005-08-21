class WigglyWidget < Qt::Widget
    slots 'setText(const QString&)'

    def initialize(parent = nil)
        super(parent)
        setBackgroundRole(Qt::Palette::Midlight)
    
        newFont = font()
        newFont.pointSize = newFont.pointSize() + 20
        setFont(newFont);
    
        @step = 0
        @timer = Qt::BasicTimer.new
        @timer.start(60, self)
    end

    def setText(newText)
        @text = newText
    end

    def paintEvent(event)
        sineTable =
        [0, 38, 71, 92, 100, 92, 71, 38,    0, -38, -71, -92, -100, -92, -71, -38]

        metrics = Qt::FontMetrics.new(font())
        x = (width() - metrics.width(@text)) / 2
        y = (height() + metrics.ascent() - metrics.descent()) / 2
        color = Qt::Color.new
    
        painter = Qt::Painter.new(self)
        (0...@text.size).each do |i|
            index = (@step + i) % 16
            color.setHsv((15 - index) * 16, 255, 191)
            painter.pen = color
            painter.drawText(x, y - ((sineTable[index] * metrics.height()) / 400),
                            @text[i, 1])
            x += metrics.width(@text[i, 1])
        end
        painter.end
    end

    def timerEvent(event)
        if event.timerId == @timer.timerId
            @step += 1
            update()
        else
            Qt::Widget.timerEvent(event)
        end
    end
end