require 'Qt'

class CannonField < Qt::Widget
	signals 'angleChanged(int)', 'forceChanged(int)'
	slots 'setAngle(int)', 'setForce(int)'
	
	def initialize(parent, name)
		super
		@ang = 45
		@f = 0
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
	
	def setForce( newton )
		if newton < 0
			newton = 0
		end
		if @f == newton
			return
		end
		@f = newton
		emit forceChanged( @f )
	end

	def paintEvent( e )
		if !e.rect().intersects( cannonRect() )
			return
		end

		cr = cannonRect()
		pix = Qt::Pixmap.new( cr.size() )
		pix.fill( self, cr.topLeft() )
		
		p = Qt::Painter.new( pix )
		p.setBrush( blue )
		p.setPen( Qt.NoPen )
		p.translate( 0, pix.height() - 1 )
		p.drawPie( Qt::Rect.new(-35, -35, 70, 70), 0, 90*16 )
		p.rotate( - @ang )
		p.drawRect( Qt::Rect.new(33, -4, 15, 8) )
		p.end()
		
		p.begin(self)
		p.drawPixmap(cr.topLeft(), pix )		
	end

	def cannonRect()
		r = Qt::Rect.new( 0, 0, 50, 50)
		r.moveBottomLeft( rect().bottomLeft() )
		return r
	end
	
	def sizePolicy()
    	return Qt::SizePolicy.new( Qt::SizePolicy.Expanding, Qt::SizePolicy.Expanding )
	end
end
