#!/usr/bin/env ruby -w

require 'Qt'

# an analog clock widget using an internal QTimer
class AnalogClock < Qt::Widget
	slots 'setTime(const QTime&)', 'drawClock(QPainter*)', 'timeout()'

	def initialize
		super

		@time = Qt::Time::currentTime
		@internalTimer = Qt::Timer.new(self)
		connect(@internalTimer, SIGNAL('timeout()'), self, SLOT('timeout()'))
		@internalTimer.start(5000)
	end

	def mousePressEvent(e)
		if isTopLevel
			topLeft = geometry.topLeft - frameGeometry.topLeft
			@clickPos = e.pos + topLeft
		end
	end

	def mouseMoveEvent(e)
		if isTopLevel
			move(e.globalPos - @clickPos) unless @clickPos.nil?
		end
	end

	def setTime(t)
		# erm. huh?
		timeout()
	end

	# The QTimer::timeout() signal is received by this slot.
	def timeout
		new_time = Qt::Time::currentTime
		@time = @time.addSecs 5
		unless new_time.minute == @time.minute
			if autoMask
				updateMask
			else
				update
			end
		end
	end

	def paintEvent(blah)
		unless autoMask
			paint = Qt::Painter.new(self)
			paint.setBrush(colorGroup.foreground)
			drawClock(paint)
		end
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

		paint.save
		paint.rotate(30*(@time.hour%12-3) + @time.minute/2)
		pts = Qt::PointArray.new(4, [-20,0, 0,-20, 300,0, 0,20])
		paint.drawConvexPolygon(pts)
		paint.restore

		paint.save
		paint.rotate((@time.minute-15)*6)
		pts = Qt::PointArray.new(4, [-10,0, 0,-10, 400,0, 0,10])
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
