#!/usr/bin/ruby

 #
 # A class that lets the user draw with the mouse. The
 # window knows how to redraw itself.
 #

require 'Qt'
	
	class ScribbleArea < Qt::Widget
	
		slots "setColor(QColor)", "slotLoad(const QString&)", "slotSave(const QString&)", "slotClearArea()"

		  #
		  # The constructor. Initializes the member variables.
		  #
		def initialize()
			super
			# initialize member variables
			@_buffer = Qt::Pixmap.new()
			@_last = Qt::Point.new()
			@_currentcolor = black()
			
			# don't blank the window before repainting
			setBackgroundMode( NoBackground )
			
			# create a pop-up menu
			@_popupmenu = Qt::PopupMenu.new()
			@_popupmenu.insertItem( "&Clear", self, SLOT( "slotClearArea()" ) )
		end
		
		  #
		  # self slot sets the curren color for the scribble area. It will be
		  # connected with the colorChanged( Qt::Color ) signal from the
		  # ScribbleWindow.
		  #
		def setColor( new_color )
			@_currentcolor = new_color
		end
		
		 #
		 # self slot clears the drawing area by filling the off-screen buffer with
		 # white and copying it over to the window.
		 #
		def slotClearArea()
			# fill the off screen buffer with plain white
			@_buffer.fill( white() )
			
			# and copy it over to the window
			Qt::PaintDevice.bitBlt( self, 0, 0, @_buffer )
		end
		
		
		  #
		  # self method does the actual loading. It relies on Qt::Pixmap (and the
		  # underlying I/O machinery) to determine the filetype.
		  #
		def slotLoad( filename )
			if !@_buffer.load( filename )
				Qt::MessageBox.warning( nil, "Load error", "Could not load file" )
			end
				
			repaint()  # refresh the window
		end
		
		
		  #
		 # self method does the actual saving. We hard-code the file type as
		 # BMP. Unix users might want to replace self with something like XPM.
		 #
		def slotSave( filename )
			if !@_buffer.save( filename, "BMP" )
				Qt::MessageBox.warning( nil, "Save error", "Could not save file" )
			end
		end
		
		
		  #
		  # self method is called whenever the user presses the
		  # mouse over the window. It just records the position of the mouse
		  # at the time of the click.
		  #
		def mousePressEvent(event)
			if event.button() == RightButton
				@_popupmenu.exec( Qt::Cursor.pos() )
			else
				@_last = event.pos()	# retrieve the coordinates from the event
			end
		end
		
		
		  #
		  # The method is called whenever the usr moves the mouse
		  # while the mouse button is pressed. If we had called
		  # setMouseTracking(true) before, the method would also be called
		  # when the mouse was moved with any button pressed. We know that
		  # we haven't, and thus don't have to check whether any buttons are
		  # pressed.
		  #
		def mouseMoveEvent(event)
			# create a Qt::Painter object for drawing onto the window
			windowpainter = Qt::Painter.new()
			# and another Qt::Painter object for drawing int an off-screen pixmap
			bufferpainter = Qt::Painter.new()
			
			# start painting
			windowpainter.begin( self ) # self painter paints onto the window
			bufferpainter.begin( @_buffer )  # and self one paints in the buffer
		
			# set a standard pen with the currently selected color
			windowpainter.setPen( @_currentcolor )
			bufferpainter.setPen( @_currentcolor )
		
			# draw a line in both the window and the buffer
			windowpainter.drawLine( @_last, event.pos() )
			bufferpainter.drawLine( @_last, event.pos() )
		
			# done with painting
			windowpainter.end()
			bufferpainter.end()
		
			# remember the current mouse position
			@_last = event.pos()						
		end
		
		  #
		  # self method is called whenever the widget needs
		  # painting, for example when it has been obscured and then revealed again.
		  #
		def paintEvent(event)
			Qt::PaintDevice.bitBlt(self, 0, 0, @_buffer)
		end
	
		  #
		  # self method get called whenever the widget needs
		  # painting, for example, when it has been obscured and then revealed again.
		  #
		def resizeEvent(event)
			save = Qt::Pixmap.new( @_buffer )
			@_buffer.resize( event.size() )
			@_buffer.fill( white() )
			Qt::PaintDevice.bitBlt( @_buffer, 0, 0, save )
		end
	end

class ScribbleWindow < Qt::Widget

	slots "slotAbout()", "slotAboutQt()", "slotColorMenu(int)", "slotLoad()", "slotSave()"
	signals "colorChanged(QColor)", "load(const QString&)", "save(const QString&)"
	
	COLOR_MENU_ID_BLACK = 0
	COLOR_MENU_ID_RED = 1
	COLOR_MENU_ID_BLUE = 2
	COLOR_MENU_ID_GREEN = 3
	COLOR_MENU_ID_YELLOW = 4
	
	def initialize()
		super
		# The next lines build the menu bar. We first create the menus
		# one by one, then add them to the menu bar. #
		@_filemenu = Qt::PopupMenu.new()  # create a file menu
		@_filemenu.insertItem( "&Load", self, SLOT( "slotLoad()" ) )
		@_filemenu.insertItem( "&Save", self, SLOT( "slotSave()" ) )
		@_filemenu.insertSeparator()
		@_filemenu.insertItem( "&Quit", $qApp, SLOT( "quit()" ) )
		
		@_colormenu = Qt::PopupMenu.new() # create a color menu
		@_colormenu.insertItem( "B&lack", COLOR_MENU_ID_BLACK)
		@_colormenu.insertItem( "&Red", COLOR_MENU_ID_RED)
		@_colormenu.insertItem( "&Blue", COLOR_MENU_ID_BLUE)
		@_colormenu.insertItem( "&Green", COLOR_MENU_ID_GREEN)
		@_colormenu.insertItem( "&Yellow", COLOR_MENU_ID_YELLOW)
		Qt::Object.connect( @_colormenu, SIGNAL( "activated( int )" ),
						 self, SLOT( "slotColorMenu( int )" ) )
						
		@_helpmenu = Qt::PopupMenu.new()  # create a help menu
		@_helpmenu.insertItem( "&About QtScribble", self, SLOT( "slotAbout()" ) )
		@_helpmenu.insertItem( "&About Qt", self, SLOT( "slotAboutQt()" ) )
		
		@_menubar = Qt::MenuBar.new( self, "" )  # create a menu bar
		@_menubar.insertItem( "&File", @_filemenu )
		@_menubar.insertItem( "&Color", @_colormenu )
		@_menubar.insertItem( "&Help", @_helpmenu )
		
		 # We create a Qt::ScrollView and a ScribbleArea. The ScribbleArea will
		 # be managed by the scroll view.#
		@_scrollview = Qt::ScrollView.new( self )
		@_scrollview.setGeometry( 0, @_menubar.height(),
		 							width(), height() - @_menubar.height() )
		@_scribblearea = ScribbleArea.new()
		@_scribblearea.setGeometry( 0, 0, 1000, 1000 )
		@_scrollview.addChild( @_scribblearea )
		Qt::Object.connect( self, SIGNAL( "colorChanged(QColor)" ),
						 @_scribblearea, SLOT( "setColor(QColor)" ) )
		Qt::Object.connect( self, SIGNAL( "save(const QString&)" ),
						 @_scribblearea, SLOT( "slotSave(const QString&)" ) )
		Qt::Object.connect( self, SIGNAL( "load(const QString&)" ),
						 @_scribblearea, SLOT( "slotLoad(const QString&)" ) )
	end
	
	def resizeEvent( event )
		 # When the whole window is resized, we have to rearrange the geometry
		 # in the ScribbleWindow as well. Note that the ScribbleArea does not need
		 # to be changed.
		 @_scrollview.setGeometry( 0, @_menubar.height(),
		 							width(), height() - @_menubar.height() )
	end

	
	
	def slotAbout()
		Qt::MessageBox.information( self, "About QtScribble 5",
										"This is the Scribble 5 application\n" +
										"Copyright 1998 by Mathias Kalle Dalheimer\n")
	end
	
	def slotAboutQt()
		Qt::MessageBox.aboutQt( self, "About Qt" )
	end
	
	def slotColorMenu( item )
		case item
			when COLOR_MENU_ID_BLACK
				emit colorChanged(black())
			when COLOR_MENU_ID_RED
				emit colorChanged(darkRed())
			when COLOR_MENU_ID_BLUE
				emit colorChanged(darkBlue())
			when COLOR_MENU_ID_GREEN
				emit colorChanged(darkGreen())
			when COLOR_MENU_ID_YELLOW
				emit colorChanged(yellow())
		end
	end
	
	
	  #
	  # self is the slot for the menu item File/Load. It opens a
	  # Qt::FileDialog to ask the user for a filename, then emits a save()
	  # signal with the filename as parameter.
	  #
	def slotLoad()
		 # Open a file dialog for loading. The default directory is the
		 # current directory, the filter *.bmp.
		 #
		filename = Qt::FileDialog.getOpenFileName( ".", "*.bmp", self )
		if !filename.nil?
			emit load(filename)
		end
	end
	
	  #
	  # self is the slot for the menu item File/Load. It opens a
	  # Qt::FileDialog to ask the user for a filename, then emits a save()
	  # signal with the filename as parameter.
	  #
	def slotSave()
		 # Open a file dialog for saving. The default directory is the
		 # current directory, the filter *.bmp.
		 #
		filename = Qt::FileDialog.getSaveFileName( ".", "*.bmp", self )
		if !filename.nil?
			emit save(filename)
		end
	end
end
	
	myapp = Qt::Application.new(ARGV)
	mywidget = ScribbleWindow.new()
	mywidget.setGeometry(50, 500, 400, 400)

	myapp.setMainWidget(mywidget)
	mywidget.show()
	myapp.exec()
