require 'Qt'

class Hello < Qt::Widget

	signals 'clicked()'
	slots 'animate()'

	# Constructs a Hello widget. Starts a 40 ms animation timer
	def initialize (text)
		super()

		@b = 0
		@text = text
		@sin_tbl = [0, 38, 71, 92, 100, 92, 71, 38,	0, -38, -71, -92, -100, -92, -71, -38]
		timer = Qt::Timer.new(self);
		connect(timer, SIGNAL('timeout()'), SLOT('animate()'))
		timer.start(40);

		resize(260, 130)
	end

	#  This slot is called each time the timer fires.
	def animate
		@b = (@b + 1) & 15
		repaint(false)
	end
	
	# Handles mouse button release events for the Hello widget.
	#
	# We emit the clicked() signal when the mouse is released inside
	# the widget.
	def mouseReleaseEvent(e)
		if (rect.contains(e.pos))
			emit clicked
		end
	end

	# Handles paint events for the Hello widget.
	#
	# Flicker-free update. The text is first drawn in the pixmap and the
	# pixmap is then blt'ed to the screen.
	def paintEvent(e)
		if @text.empty?
			return
		end

		# 1: Compute some sizes, positions etc.
		fm = fontMetrics

		w = fm.width(@text) + 20
		h = fm.height * 2
		pmx = width/2 - w/2
		pmy = height/2 - h/2

		# 2: Create the pixmap and fill it with the widget's background
		pm = Qt::Pixmap.new(w, h)
		pm.fill(self, pmx, pmy)

		# 3: Paint the pixmap. Cool wave effect
		p = Qt::Painter.new;
		x = 10
		y = h/2 + fm.descent
		i = 0
		p.begin(pm)
		#p.begin(self)
		#p.eraseRect(0,0, width, height)
		p.setFont(font)

		for i in 0..@text.size-1
			j = (@b+i) & 15
			p.setPen(Qt::Color.new((15-j)*16,255,255,Qt::Color.Hsv) )
			p.drawText( x, y-@sin_tbl[j]*h/800, @text[i,1], 1 )
			x += fm.width(@text[i,1])
		end
		p.end

		# We do this since bitBlt is not a QPaintDevice member?!?! rather
		# it's a friend function.. (thanks guys)
		# so we don't actually have bitBlt support, but here's how it should work
		# and here's what we do instead
		begin
			#4: Copy the pixmap to the Hello widget
			bitBlt(self, pmx, pmy, pm)
		rescue
			p = Qt::Painter.new
			p.begin(self)
			p.drawPixmap(pmx,pmy, pm)
			p.end
		end

	end
end
