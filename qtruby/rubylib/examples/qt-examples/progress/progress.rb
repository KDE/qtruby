#!/usr/bin/ruby -w

require 'Qt'

class AnimatedThingy < Qt::Label
    attr_accessor :label, :step
    attr_accessor :ox0, :oy0, :ox1, :oy1
    attr_accessor :x0,  :y0,  :x1,  :y1
    attr_accessor :dx0, :dx1, :dy0, :dy1
    NQIX = 10

    def initialize(*k)
        super(*k)
        @label = k[1] + "\n... and wasting CPU\nwith this animation!\n"
        @step = 0
        @ox0, @oy0, @ox1, @oy1 = *Array.new(4) { Array.new(10, 0) }
        @x0 = @y0 = @x1 = @y1 = 0
        @dx0 = rand(8)+2
        @dy0 = rand(8)+2
        @dx1 = rand(8)+2
        @dy1 = rand(8)+2
    end

    def show
        startTimer(150) unless isVisible
        super
    end

    def hide
	super
	killTimers()
    end

    def sizeHint
	Qt::Size.new(120,100)
    end

    def inc(x, dx, b)
	x += dx
	if x < 0
            x = 0
            dx = rand(8) + 2
        elsif x >= b
            x = b-1
            dx = -(rand(8)+2) 
        end
	yield x, dx
    end

    def timerEvent(e)
	painter = Qt::Painter.new(self)
	pn = painter.pen
	pn.setWidth(2)
	pn.setColor(backgroundColor)
	painter.setPen(pn)

	@step = (@step + 1) % NQIX

	painter.drawLine(@ox0[@step], @oy0[@step], @ox1[@step], @oy1[@step])

	inc(@x0, @dx0, width)  { |x,dx| @x0, @dx0 = x, dx }
	inc(@y0, @dy0, height) { |y,dy| @y0, @dy0 = y, dy }
	inc(@x1, @dx1, width)  { |x,dx| @x1, @dx1 = x, dx }
	inc(@y1, @dy1, height) { |y,dy| @y1, @dy1 = y, dy }
	@ox0[@step] = @x0
	@oy0[@step] = @y0
	@ox1[@step] = @x1
	@oy1[@step] = @y1

	c = Qt::Color.new
	c.setHsv( (@step*255)/NQIX, 255, 255 ) # rainbow effect
	pn.setColor(c)
	painter.setPen(pn)
	painter.drawLine(@ox0[@step], @oy0[@step], @ox1[@step], @oy1[@step])
	painter.setPen(white)
	painter.drawText(rect(), AlignCenter, @label)
    end
end

class CPUWaster < Qt::Widget
    attr_accessor :menubar, :file, :options, :rects, :pb
    attr_accessor :td_id , :ld_id, :dl_id, :cl_id, :md_id
    attr_accessor :got_stop, :timer_driven, :default_label
    slots 'drawItemRects(int)', 'doMenuItem(int)', 'stopDrawing()', 'timerDriven()'
    slots 'loopDriven()', 'defaultLabel()', 'customLabel()', 'toggleMinimumDuration()'

    FIRST_DRAW_ITEM = 1000
    LAST_DRAW_ITEM  = 1006

    def initialize(*k)
        super(*k)

        @menubar = Qt::MenuBar.new(self, "menu")
        @pb = nil

        @file = Qt::PopupMenu.new
        @menubar.insertItem( "&File", file )
	FIRST_DRAW_ITEM.upto(LAST_DRAW_ITEM) { 
	    |i| file.insertItem(  "#{drawItemRects(i)} Rectangles", i) 
	}
        connect( menubar, SIGNAL('activated(int)'), self, SLOT('doMenuItem(int)') )
        @file.insertSeparator
        @file.insertItem("Quit", $qApp, SLOT('quit()'))
        @options = Qt::PopupMenu.new
        @menubar.insertItem("&Options", options)
        @td_id = options.insertItem("Timer driven",  self, SLOT('timerDriven()'))
        @ld_id = options.insertItem("Loop driven",   self, SLOT('loopDriven()'))
        @options.insertSeparator
        @dl_id = options.insertItem("Default label", self, SLOT('defaultLabel()'))
        @cl_id = options.insertItem("Custom label",  self, SLOT('customLabel()'))
        @options.insertSeparator
        @md_id = options.insertItem("No minimum duration", self, SLOT('toggleMinimumDuration()'))
        @options.setCheckable true

	loopDriven
	defaultLabel

        setFixedSize(400, 300)
        setBackgroundColor(white)
    end

    def drawItemRects(id)
        n = id - FIRST_DRAW_ITEM
        r = 100
	n.downto(0) {
	    everythree = (n % 3) != 0
	    r *= everythree ? 5 : 4
	}
        r
    end

    def doMenuItem(id)
        draw drawItemRects(id) if id >= FIRST_DRAW_ITEM && id <= LAST_DRAW_ITEM
    end

    def stopDrawing
        @got_stop = 1 
    end

    def timerDriven
        @timer_driven = true
        @options.setItemChecked(@td_id, true)
        @options.setItemChecked(@ld_id, false)
    end

    def loopDriven
        @timer_driven = false
        @options.setItemChecked(@td_id, false)
        @options.setItemChecked(@ld_id, true)
    end

    def defaultLabel
        @default_label = true
        @options.setItemChecked(@dl_id, true)
        @options.setItemChecked(@cl_id, false)
    end

    def customLabel
        @default_label = false
        @options.setItemChecked(@dl_id, false)
        @options.setItemChecked(@cl_id, true)
    end

    def toggleMinimumDuration
        checked = @options.isItemChecked(@md_id)
        @options.setItemChecked(@md_id, !checked)
    end

    def timerEvent(e)
        @pb.setProgress( @pb.totalSteps - @rects ) if @rects % 100 == 0
        @rects -= 1

	painter = Qt::Painter.new(self)

	ww = width
	wh = height

	if ww > 8 and wh > 8
	    c = Qt::Color(rand(255), rand(255), rand(255))
	    x = rand(ww - 8)
	    y = rand(wh - 8)
	    w = rand(ww - x)
	    h = rand(wh - y)
	    painter.fillRect(x, y, w, h, Qt::Brush.new(c))
	end

        if @rects < 0 || @got_stop
            @pb.setProgress(@pb.totalSteps)
            painter = Qt::Painter.new(self)
            painter.fillRect(0, 0, width(), height(), Qt::Brush.new(backgroundColor))
            enableDrawingItems(true)
            killTimers()
            @pb = nil
        end
    end

    def newProgressDialog(label, steps, modal)
        d = Qt::ProgressDialog.new(label, "Cancel", steps, self, "progress", modal)
        d.setMinimumDuration(true) if @options.isItemChecked(@md_id)
        d.setLabel( AnimatedThingy.new(d, label) ) unless @default_label
	d.show
        d
    end

    def enableDrawingItems(yes)
        FIRST_DRAW_ITEM.upto(LAST_DRAW_ITEM) {
            |i| menubar.setItemEnabled(i, yes)
        }
    end

    def draw(n)
        if timer_driven
            if @pb.nil?
                warn("This cannot happen!")
                return
            end
            @rects = n
            @pb = newProgressDialog("Drawing rectangles.\nUsing timer event.", n, false)
            @pb.setCaption("Please Wait")
            connect(@pb, SIGNAL('cancelled()'), self, SLOT('stopDrawing()'))
            enableDrawingItems(0)
            startTimer(0)
            @got_stop = 0
        else
            lpb = newProgressDialog("Drawing rectangles.\nUsing loop.", n, true)
            lpb.setCaption("Please Wait")

            painter = Qt::Painter.new(self)
            0.upto(n) { |i|
		if (i % 100) == 0
		    lpb.setProgress(i)
                    break if lpb.wasCancelled
                end
                cw, ch = width, height
                c = Qt::Color.new(rand(255), rand(255), rand(255))
                x = rand(cw - 8)
                y = rand(cw - 8)
                w = rand(cw - x)
                h = rand(cw - y)
                painter.fillRect(x, y, w, h, Qt::Brush.new(c))
            }
            lpb.cancel
            painter.fillRect(0, 0, width, height, Qt::Brush.new(backgroundColor))
        end
    end
end

a = Qt::Application.new(ARGV)
w = CPUWaster.new
w.show
a.setMainWidget(w)
a.exec
