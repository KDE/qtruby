#
# Copyright (c) 2001 by Jim Menard <jimm@io.com>
#
# Released under the same license as Ruby. See
# http://www.ruby-lang.org/en/LICENSE.txt.
#

require 'Qt'
require 'Canvas'
require 'CameraDialog'

class WorldWindow < Qt::MainWindow
	slots 'slotMenuActivated(int)'
	
    MENU_CAMERA_DIALOG = 1

    attr_accessor :canvas

    def initialize
	super
	setCaption("Boids")
	setupMenubar()

	@canvas = Canvas.new(self, "TheDamnCanvas")
	setCentralWidget(@canvas)
	setGeometry(0, 0, $PARAMS['window_width'],
		    $PARAMS['window_height'])
    end

    def setupMenubar

	# Create and populate file menu
    menu = Qt::PopupMenu.new(self)
    menu.insertItem("Exit", $qApp, SLOT("quit()"), Qt::KeySequence.new(CTRL+Key_Q))

	# Add file menu to menu bar
	menuBar.insertItem("&File", menu)

	# Create and populate options menu
	menu = Qt::PopupMenu.new(self)
	menu.insertItem("&Camera...", MENU_CAMERA_DIALOG, -1)

	# Add options menu to menu bar and link it to method below
	menuBar.insertItem("&Options", menu)
	connect(menu, SIGNAL("activated(int)"), self, SLOT('slotMenuActivated(int)'))

    end

    def slotMenuActivated(id)
	if id == MENU_CAMERA_DIALOG
	    CameraDialog.new(nil).exec()
	end
    end
end
