require 'Qt'

class DynamicTip < Qt::ToolTip
	def initialize(p)
		super(p)
	end

	def maybeTip (p)
		if !parentWidget.inherits('TellMe')
			return
		end

		r = parentWidget.tip(p)
		if !r.isValid
			return
		end

		s = 'position: ' + r.center.x.to_s + ', ' + r.center.y.to_s
		tip(r,s)
	end
end

class TellMe < Qt::Widget

	def initialize
		super

		setMinimumSize(30, 30)

		@r1 = randomRect
		@r2 = randomRect
		@r3 = randomRect

		@t = DynamicTip.new(self)

		Qt::ToolTip.add(self, @r3, 'this color is called red') #TT says this is helpful, I'm not so sure
	end

	def tip(point)
		if (@r1.contains(point))
			@r1
		elsif (@r2.contains(point))
			@r2
		else
			Qt::Rect.new(0,0, -1, -1)
		end
	end

	def paintEvent(e)
		p = Qt::Painter.new(self)

		if (e.rect.intersects(@r1))
			p.setBrush(Qt::blue)
			p.drawRect(@r1)
		end

		if (e.rect.intersects(@r2))
			p.setBrush(Qt::blue)
			p.drawRect(@r2)
		end

		if (e.rect.intersects(@r3))
			p.setBrush(Qt::red)
			p.drawRect(@r3)
		end
	end

	def mousePressEvent (e)
		if (@r1.contains(e.pos))
			@r1 = randomRect
		end

		if (@r2.contains(e.pos))
			@r2 = randomRect
		end
		
		repaint
	end

	def resizeEvent(e)
		unless rect.contains(@r1)
			@r1 = randomRect
		end

		unless rect.contains(@r2)
			@r2 = randomRect
		end
	end

	def randomRect
		Qt::Rect.new(rand(width - 20), rand(height - 20), 20, 20)
	end
end
