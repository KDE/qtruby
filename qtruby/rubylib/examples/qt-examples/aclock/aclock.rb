#!/usr/bin/ruby -w

require 'Qt'

# HACK
def new_point_array(a)
    pa = Qt::PointArray.new(a.length / 2)
    p a.length / 2
    0.upto((a.length / 2) - 1) {
        |i| 
        p i
        p a[i*2], a[(i*2)+1]
        pa.setPoint(i, a[i*2], a[(i*2)+1])
    }
    pa
end

# an analog clock widget using an internal QTimer
#
class AnalogClock < Qt::Widget
    slots 'setTime(const QTime&)', 'drawClock(QPainter*)', 'timeout()'
    attr_accessor :clickPos, :_time

    def initialize(*k)
	super(*k)
	@_time = Qt::Time::currentTime
	@internalTimer = Qt::Timer.new(self)
	connect(@internalTimer, SIGNAL('timeout()'), self, SLOT('timeout()'))
	@internalTimer.start(5000)
    end

    def mousePressEvent(e)
	return unless isTopLevel
	topLeft = geometry.topLeft - frameGeometry.topLeft
	@clickPos = e.pos + topLeft
    end

    def mouseMoveEvent(e)
	return unless isTopLevel
	move(e.globalPos - @clickPos) unless @clickPos.nil?
    end

    def setTime(t)
	# erm. huh?
	timeout()
    end

    # The QTimer::timeout() signal is received by this slot.
    def timeout
	new_time = Qt::Time::currentTime
	@_time = @_time.addSecs 5
	return if new_time.minute == @_time.minute
        if autoMask then updateMask else update end
    end

    def paintEvent(blah)
	return if autoMask
	paint = Qt::Painter.new(self)
	paint.setBrush(colorGroup.foreground)
	drawClock(paint)
    end

    # If clock is transparent, we use updateMask() instead of paintEvent()
    def updateMask
	bm = Qt::Bitmap.new(size)
	bm.fill(color0)			# transparent

	paint = Qt::Painter.new
	paint.begin(bm, self)
	paint.setBrush(color1)		# use non-transparent color
	paint.setPen(color1)

	drawClock(paint)

	paint.end
	setMask(bm)
    end

    # The clock is painted using a 1000x1000 square coordinate system, in
    # the centered square, as big as possible.  The painter's pen and
    # brush colors are used.
    def drawClock(paint)
	paint.save

	paint.setWindow(-500,-500, 1000,1000)

	v = paint.viewport
	d = [v.width, v.height].min
	vpx = (v.left + (v.width-d)) / 2
	vpy = (v.top  - (v.height-d)) / 2
	paint.setViewport(vpx, vpy, d, d) 
	# @_time = Qt::Time::currentTime(); huh ?

	paint.save
	paint.rotate(30*(@_time.hour%12-3) + @_time.minute/2)
	pts = new_point_array([-20,0, 0,-20, 300,0, 0,20])
	paint.drawConvexPolygon(pts)
	paint.restore

	paint.save
	paint.rotate((@_time.minute-15)*6)
	pts = new_point_array([-10,0, 0,-10, 400,0, 0,10])
	paint.drawConvexPolygon(pts)
	paint.restore;

	12.times {
	    paint.drawLine(440,0, 460,0)
	    paint.rotate(30)
	}

	paint.restore
    end

    def setAutoMask(background)
	setBackgroundMode(background ? PaletteForeground : PaletteBackground)
	Qt::Widget::setAutoMask(background)
    end

end

a = Qt::Application.new(ARGV)
clock = AnalogClock.new
clock.setAutoMask(true) if ARGV[0] == '-transparent'
clock.resize(100, 100)
a.setMainWidget(clock)
clock.setCaption("PerlQt example - Analog Clock")
clock.show
a.exec
