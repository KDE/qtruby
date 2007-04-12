=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end


=begin
This template constructs an application with menus, toolbar and statusbar,
HOWEVER it is not recommended this template actually be used. It presents
the "KDE 1.0" method for constructing menus and toolbars - later versions
of KDE have introduced better(easier and more powerful) methods for
doing this job - see other menuapp*.rb templates for these methods
=end

=begin
Copyright 2003 Jim Bublitz

Terms and Conditions

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KDE::IND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Except as contained in this notice, the name of the copyright holder shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from the
copyright holder.
=end


require 'Korundum'

class MainWin < KDE::MainWindow
	TOOLBAR_NEW   = 1
	TOOLBAR_OPEN  = 2
	TOOLBAR_SAVE  = 3
	TOOLBAR_CUT   = 4
	TOOLBAR_COPY  = 5
	TOOLBAR_PASTE = 6

	STATUSBAR_LEFT   = 1
	STATUSBAR_MIDDLE = 2
	STATUSBAR_RIGHT  = 3
	
	slots	'slotNew()', 'slotOpen()', 'slotSave()', 'slotSaveAs()', 'slotPrint()', 'slotQuit()', 'slotUndo()',
 			'slotRedo()', 'slotCut()', 'slotCopy()', 'slotPaste()', 'slotFind()', 'slotFindNext()', 'slotReplace()',
			'slotSpecial()'
	
    def initialize(*args)
        super

        initMenus()
        initToolBar()
        initStatusBar()
	end

    def initMenus()
        # Create a QPopupMenu - all menus are "popup" menus
        
		fileMenu = Qt::PopupMenu.new(self)
        
		# This is the "simple" KDE-1.0 way. It is not suggested that this
        # template actually be used in an application, but it's
        # provided to show the underlying mechanics of menu construction
        # that KDE makes much easier with other methods (see other
        # menuapp*.rb templates for usable examples)

        # All menu item strings are wrapped with i18n - this allows
        # internationalization

        # Predefined accelerators are in KDE::StdAccel - these are
        # the standard accelerators. For custom accelerators, use
        # KDE::Accel. All KDE::StdAccel methods are static, so there is no
        # need to instantiate KDE::StdAccel
		        
		# "File" menu items
        fileMenu.insertItem(i18n("New"), self, SLOT('slotNew()'), Qt::KeySequence.new(KDE::StdAccel.openNew().keyCodeQt()))
        fileMenu.insertItem(i18n("Open"), self, SLOT('slotOpen()'), Qt::KeySequence.new(KDE::StdAccel.open().keyCodeQt()))
        fileMenu.insertSeparator()
        fileMenu.insertItem(i18n("Save"), self, SLOT('slotSave()'), Qt::KeySequence.new(KDE::StdAccel.save().keyCodeQt()))
        
		# KStdAccel doesn't have a standard accelerator for 'Save As',
        # so we omit it - insertItem uses the default value
 		fileMenu.insertItem(i18n("SaveAs"), self, SLOT('slotSaveAs()'))
        
		# This inserts a line between groups of items in a menu

        fileMenu.insertSeparator()
        fileMenu.insertItem(i18n("Print"), self, SLOT('slotPrint()'), Qt::KeySequence.new(KDE::StdAccel.print().keyCodeQt()))
        fileMenu.insertSeparator()
        fileMenu.insertItem(i18n("&Quit"), self, SLOT('slotQuit()'), Qt::KeySequence.new(KDE::StdAccel.quit().keyCodeQt()))

        # Put fileMenu (as the File menu) into the menu bar
        # 'menuBar' is a predefined object owned by KDE::MainWindow
        menuBar().insertItem(i18n("&File"), fileMenu)

        editMenu = Qt::PopupMenu.new(self)

        # "Edit" menu items
        editMenu.insertItem(i18n("Undo"), self, SLOT('slotUndo()'), Qt::KeySequence.new(KDE::StdAccel.undo().keyCodeQt()))
        editMenu.insertItem(i18n("Redo"), self, SLOT('slotRedo()'), Qt::KeySequence.new(KDE::StdAccel.redo().keyCodeQt()))
        editMenu.insertSeparator()
        editMenu.insertItem(i18n("Cut"), self, SLOT('slotCut()'), Qt::KeySequence.new(KDE::StdAccel.cut().keyCodeQt()))
        editMenu.insertItem(i18n("Copy"), self, SLOT('slotCopy()'), Qt::KeySequence.new(KDE::StdAccel.copy().keyCodeQt()))
        editMenu.insertItem(i18n("Paste"), self, SLOT('slotPaste()'), Qt::KeySequence.new(KDE::StdAccel.paste().keyCodeQt()))
        editMenu.insertSeparator()
        editMenu.insertItem(i18n("Find"), self, SLOT('slotFind()'), Qt::KeySequence.new(KDE::StdAccel.find().keyCodeQt()))
        editMenu.insertItem(i18n("Find Next"), self, SLOT('slotFindNext()'), Qt::KeySequence.new(KDE::StdAccel.findNext().keyCodeQt()))
        editMenu.insertItem(i18n("Replace"), self, SLOT('slotReplace()'), Qt::KeySequence.new(KDE::StdAccel.replace().keyCodeQt()))

        # Put editMenu (as the Edit menu) into the menu bar
		
        menuBar().insertItem(i18n("&Edit"), editMenu)
        
		# Let KDE generate a nifty help menu

        # The KDE::AboutData/KDE::CmdLineArgs data from the main part of the program
        # will be used to generate the About dialog
        helpMenu = helpMenu("")
        menuBar().insertItem(i18n("&Help"), helpMenu)
	end

    def initToolBar()
        # KDE::IconLoader will make it easy to locate the standard KDE icons for
        # toolbar buttons. For custom icons, a complete path to the icon
        # (without the loadIcon call) is needed
        icons = KDE::IconLoader.new()

        # KDE::MainWindow owns at least one KDE::ToolBar instance, which is returned
        # by 'toolBar()'. To obtain additional toolbars, add an argument
        # to the call -- toolBar(1) will return another toolbar you can
        # add buttons to.

        # Add buttons to the toolbar. The icon name, id value (eg TOOLBAR_NEW),
        # signal to connect (eg clicked) and the slot to connect to all need
        # to be specified,as does the tooltip (the last string argument). There
        # are easier ways to do this - see other menuapp templates for easier
        # methods using KDE::Action/KDE::StdAction
        
		toolBar().insertButton(icons.loadIcon("filenew", KDE::Icon::Toolbar), TOOLBAR_NEW, SIGNAL("clicked(int)"), 
                                               self, SLOT('slotNew()'), true, "New")
        toolBar().insertButton(icons.loadIcon("fileopen", KDE::Icon::Toolbar), TOOLBAR_OPEN, SIGNAL("clicked(int)"),
                                               self, SLOT('slotOpen()'), true, "Open")
        toolBar().insertButton(icons.loadIcon("filesave", KDE::Icon::Toolbar), TOOLBAR_SAVE, SIGNAL("clicked(int)"),
                                               self, SLOT('slotSave()'), true, "Save")
        toolBar().insertButton(icons.loadIcon("editcut", KDE::Icon::Toolbar), TOOLBAR_CUT, SIGNAL("clicked(int)"),
                                               self, SLOT('slotCut()'), true, "Cut")
        toolBar().insertButton(icons.loadIcon("editcopy", KDE::Icon::Toolbar), TOOLBAR_COPY, SIGNAL("clicked(int)"),
                                               self, SLOT('slotCopy()'), true, "Copy")
        toolBar().insertButton(icons.loadIcon("editpaste", KDE::Icon::Toolbar), TOOLBAR_PASTE, SIGNAL("clicked(int)"),
                                               self, SLOT('slotPaste()'), true, "Paste")
	end

    def initStatusBar()
        # KDE::MainWindow also owns a KDE::StatusBar instance. The first
        # call creates a KDE::StatusBar instance. See 'notImpl' below
        # for an example of writing to the status bar. You can
        # also add widgets (labels, progress bars, etc) to the
        # status bar
        
		statusBar().insertItem("", STATUSBAR_LEFT, 1000, true)
        statusBar().insertItem("", STATUSBAR_MIDDLE, 1000, true)
        statusBar().insertItem("", STATUSBAR_RIGHT, 1000, true)
	end


#-------------------- slots -----------------------------------------------
    
	# Slots which can be called from both the menu toolbar
    # have a second parameter with a default value (id = -1)
    # This is because menu signals expect to connect to a
    # slot that takes no arguments, while toolbar signals
    # expect to send a signal with an int argument for the
    # id of the toolbar button. The default value allows
    # both cases to work.

    def slotNew(id = -1)
        notImpl("New")
	end

    def slotOpen(id = -1)
        notImpl("Open")
	end

    def slotSave(id = -1)
        notImpl("Save")
	end

    def slotSaveAs()
        notImpl("Save As")
	end

    def slotPrint()
        notImpl("Print")
	end

    def slotQuit()
        notImpl("Qt::uit")
	end

    def slotUndo()
        notImpl("Undo")
	end

    def slotRedo()
        notImpl("Redo")
	end

    def slotCut(id = -1)
        notImpl("Cut")
	end

    def slotCopy(id = -1)
        notImpl("Copy")
	end

    def slotPaste(id = -1)
        notImpl("Paste")
	end

    def slotFind()
        notImpl("Find")
	end

    def slotFindNext()
        notImpl("Find Next")
	end

    def slotReplace()
        notImpl("Replace")
	end

    def notImpl(item = "Feature")
        statusBar().changeItem("#{item} not implemented", STATUSBAR_LEFT)
        KDE::MessageBox.error(self, "#{item} not implemented", "Not Implemented")
        statusBar().changeItem("", STATUSBAR_LEFT)
	end
end

#-------------------- main ------------------------------------------------

# See athe minimal.rb and basicapp.rb templates for
# explantion of the basic app and main window setup

# The following data is passed to KDE::CmdLineArgs, which in
# turn makes it available to the "about" box in the Help
# menu (when the Help menu is created as above)

description = "A basic application template"
version     = "1.0"
aboutData   = KDE::AboutData.new("", "",
    version, description, KDE::AboutData::License_GPL,
    "(C) 2003 whoever the author is")

aboutData.addAuthor("author1", "whatever they did", "email@somedomain")
aboutData.addAuthor("author2", "they did something else", "another@email.address")

KDE::CmdLineArgs.init(ARGV, aboutData)

KDE::CmdLineArgs.addCmdLineOptions([["+files", "File to open", ""]])

app = KDE::Application.new()
mainWindow = MainWin.new(nil, "main window")
mainWindow.show
app.exec
