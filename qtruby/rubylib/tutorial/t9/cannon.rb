require 'Qt'

class CannonField < Qt::Widget
	signals "angleChanged(int)"
	slots "setAngle(int)"
	
	def initialize(parent, name)
		super
		@ang = 45
    	setPalette( Qt::Palette.new( Qt::Color.new( 250, 250, 200) ) )
	end

	def setAngle( degrees )
		if degrees < 5
			degrees = 5
		elsif degrees > 70
        	degrees = 70
		end
		if @ang == degrees
			return
		end
		@ang = degrees
		repaint()
		emit angleChanged( @ang )
	end

	def paintEvent( event )
		p = Qt::Painter.new( self )

		p.setBrush( blue )
		p.setPen( Qt.NoPen )
		p.translate( 0, rect().bottom() )
		p.drawPie( Qt::Rect.new(-35, -35, 70, 70), 0, 90*16 )
		p.rotate( - @ang )
		p.drawRect( Qt::Rect.new(33, -4, 15, 8) )		
	end


	def sizePolicy()
    	return Qt::SizePolicy.new( Qt::SizePolicy.Expanding, Qt::SizePolicy.Expanding )
	end
end
