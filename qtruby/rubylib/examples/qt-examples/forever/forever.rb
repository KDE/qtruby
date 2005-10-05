#!/usr/bin/env ruby -w

require 'Qt'

#
# Forever - a widget that draws rectangles forever.
#

class Forever < Qt::Widget

	NUM_COLORS = 120
	#
	# Constructs a Forever widget.
	#
	
	slots 'updateCaption()'
	
	def initialize(*k)
		super(nil)
		@colors = []
		0.upto(NUM_COLORS-1) do |a|
			@colors[a] = Qt::Color.new( rand(255),
					rand(255),
					rand(255) )
		end
		@rectangles = 0
		startTimer( 0 )				# run continuous timer
		counter = Qt::Timer.new( self )
		connect( counter, SIGNAL("timeout()"),
			self, SLOT("updateCaption()") )
		counter.start( 1000 )
	end


	def updateCaption()
		s = "Qt Example - Forever - " + @rectangles.to_s + " rectangles/second"
		@rectangles = 0
		self.caption = s
	end


	#
	# Handles paint events for the Forever widget.
	#

	def paintEvent( e  )
		paint = Qt::Painter.new( self )			# painter object
		w = width()
		h = height()
		if w <= 0 || h <= 0 then
			return
		end
		paint.setPen( NoPen )			# do not draw outline
		paint.setBrush( @colors[rand(NUM_COLORS)]) # set random brush color

		p1 = Qt::Point.new( rand(w), rand(h))	# p1 = top left
		p2 = Qt::Point.new( rand(w), rand(h))	# p2 = bottom right

		r = Qt::Rect.new( p1, p2 )
		paint.drawRect( r )			# draw filled rectangle
		paint.end()
	end

	#
	# Handles timer events for the Forever widget.
	#

	def timerEvent( e )
		0.upto(99) do |i|
			repaint( false )			# repaint, don't erase
		end
		@rectangles += 100
	end

	
end

a = Qt::Application.new(ARGV)
always = Forever.new
always.resize( 400, 250 )			# start up with size 400x250
a.mainWidget = always			# set as main widget
always.caption = "QtRuby Example - Forever"
always.show
a.exec
