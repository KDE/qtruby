require 'Qt'
include Math

class CannonField < Qt::Widget
	
	signals "angleChanged(int)", "forceChanged(int)"
	slots "setAngle(int)", "setForce(int)", "shoot()", "moveShot()"
	
	def initialize(parent, name)
		super
		@ang = 45
		@f = 0
		@timerCount = 0;
        @autoShootTimer = Qt::Timer.new( self, "movement handler" )
        connect( @autoShootTimer, SIGNAL('timeout()'),
                 self, SLOT('moveShot()') );
        @shoot_ang = 0
        @shoot_f = 0    	
		setPalette( Qt::Palette.new( Qt::Color.new( 250, 250, 200) ) )
		@barrelRect = Qt::Rect.new(33, -4, 15, 8)
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
		repaint( cannonRect(), false )
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
	
	def shoot()
		if @autoShootTimer.isActive()
			return
		end;
		@timerCount = 0
		@shoot_ang = @ang
		@shoot_f = @f
		@autoShootTimer.start( 50 )
	end

	def moveShot()
		r = Qt::Region.new( shotRect() )
		@timerCount += 1

		shotR = Qt::Rect.new( shotRect() )

		if shotR.x() > width() || shotR.y() > height()
			@autoShootTimer.stop()
		else
			r = r.unite( Qt::Region.new( shotR ) )
		end
		repaint( r )
	end

	def paintEvent( e )
		updateR = e.rect()
		p = Qt::Painter.new( self )

		if updateR.intersects( cannonRect() )
			paintCannon( p )
		end
		if @autoShootTimer.isActive() &&
			updateR.intersects( shotRect() ) 
			paintShot( p )
		end
	end

	def paintShot( p )
		p.setBrush( black )
		p.setPen( Qt.NoPen )
		p.drawRect( shotRect() )
	end
	
	def paintCannon(p)				
		cr = cannonRect()
		pix = Qt::Pixmap.new( cr.size() )
		pix.fill( self, cr.topLeft() )
		
		tmp = Qt::Painter.new( pix )
		tmp.setBrush( blue )
		tmp.setPen( Qt.NoPen )
		tmp.translate( 0, pix.height() - 1 )
		tmp.drawPie( Qt::Rect.new(-35, -35, 70, 70), 0, 90*16 )
		tmp.rotate( - @ang )
		tmp.drawRect( @barrelRect )
		tmp.end()
		
		p.drawPixmap(cr.topLeft(), pix )		
	end

	def cannonRect()
		r = Qt::Rect.new( 0, 0, 50, 50)
		r.moveBottomLeft( rect().bottomLeft() )
		return r
	end
	
	def shotRect()
		gravity = 4.0

		time      = @timerCount / 4.0
		velocity  = @shoot_f
		radians   = @shoot_ang*3.14159265/180.0

		velx      = velocity*cos( radians )
		vely      = velocity*sin( radians )
		x0        = ( @barrelRect.right()  + 5.0 )*cos(radians)
		y0        = ( @barrelRect.right()  + 5.0 )*sin(radians)
		x         = x0 + velx*time
		y         = y0 + vely*time - 0.5*gravity*time*time

		r = Qt::Rect.new( 0, 0, 6, 6 );
		r.moveCenter( Qt::Point.new( x.round, height() - 1 - y.round ) )
		return r
	end

	def sizePolicy()
    	return Qt::SizePolicy.new( Qt::SizePolicy.Expanding, Qt::SizePolicy.Expanding )
	end
end
