#!/usr/bin/env ruby -w

require 'Qt'

# an analog clock widget using an internal QTimer
class AnalogClock < Qt::Widget

    def initialize(parent = nil)
        super(parent)

        @timer = Qt::Timer.new(self)
        connect(@timer, SIGNAL('timeout()'), self, SLOT('update()'))
        @timer.start(1000)

        setWindowTitle(tr("Analog Clock"))
        resize(200, 200)
    end

    def paintEvent(e)
        hourHand = Qt::Polygon.new( [   Qt::Point.new(7, 8),
                                        Qt::Point.new(-7, 8),
                                        Qt::Point.new(0, -40) ] )
        minuteHand = Qt::Polygon.new(   [   Qt::Point.new(7, 8),
                                            Qt::Point.new(-7, 8),
                                            Qt::Point.new(0, -70) ] )
        hourColor = Qt::Color.new(127, 0, 127)
        minuteColor = Qt::Color.new(0, 127, 127, 191)

        side = width() < height() ? width() : height()
        time = Qt::Time.currentTime

        painter = Qt::Painter.new(self)
        painter.setRenderHint(Qt::Painter::Antialiasing)
        painter.translate(width() / 2, height() / 2)
        painter.scale(side / 200.0, side / 200.0)

        painter.pen = Qt::NoPen
        painter.brush = Qt::Brush.new(hourColor)

        painter.save
        painter.rotate(30.0 * ((time.hour + time.minute / 60.0)))
        painter.drawConvexPolygon(hourHand)
        painter.restore

        painter.pen = hourColor
        (0...12).each do |i|
            painter.drawLine(88, 0, 96, 0)
            painter.rotate(30.0)
        end

        painter.pen = Qt::NoPen
        painter.brush = Qt::Brush.new(minuteColor)

        painter.save
        painter.rotate(6.0 * (time.minute + time.second / 60.0))
        painter.drawConvexPolygon(minuteHand)
        painter.restore

        painter.pen = minuteColor
        (0...60).each do |j|
            if (j % 5) != 0
                painter.drawLine(92, 0, 96, 0)
            end
            painter.rotate(6.0)
        end

		painter.end
    end
end
