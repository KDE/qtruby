
=begin
This template constructs an application with menus, toolbar and statusbar.
It uses KDE classes and methods that simplify the task of building and
operating a GUI. It is recommended that this approach be used, rather
than the primitive approach in menuapp1.rb
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
	STATUSBAR_LEFT   = 1
	STATUSBAR_MIDDLE = 2
	STATUSBAR_RIGHT  = 3
	
	slots	'slotNew()', 'slotOpen()', 'slotSave()', 'slotSaveAs()', 'slotPrint()', 'slotQuit()', 'slotUndo()',
 			'slotRedo()', 'slotCut()', 'slotCopy()', 'slotPaste()', 'slotFind()', 'slotFindNext()', 'slotReplace()',
			'slotSpecial()'
	
    def initialize(*args)
        super

        # Create the actions that will populate
        # the menus and toolbars
        initActions()

        # Plug actions into menus
        initMenus()

        # Plug actions into toolbars
        initToolBar()

        # Create the status bar
        initStatusBar()

        # Usings actions, only a single line is required
        # to enable/disable both the menu item and corresponding
        # toolbar button from anywhere in the program
        @saveAction.setEnabled(false)
        @saveAsAction.setEnabled(false)
	end

    def initActions()
        # Most of the functions selectable by menu are "standard"
        # actions (open a file, cut, paste, etc) - you customize
        # how they behave in your code, but menu, toolbar, and
        # accelerator settings are the same across all programs.
        # Standard actions also have tooltips already assigned

        # To create most of the actions below, KDE::StdAction is
        # is used, since it takes care of everything with
        # a single line of code.

        # The standard actions only need to specify the slot
        # where the code for the action is located

        # "File" menu items
        @newAction    = KDE::StdAction.openNew(self, SLOT("slotNew()"), actionCollection())
        @openAction   = KDE::StdAction.open(self, SLOT("slotOpen()"), actionCollection())
        @saveAction   = KDE::StdAction.save(self, SLOT("slotSave()"), actionCollection())
        @saveAsAction = KDE::StdAction.saveAs(self, SLOT("slotSaveAs()"), actionCollection())
        @printAction  = KDE::StdAction.print(self, SLOT("slotPrint()"), actionCollection())
        @quitAction   = KDE::StdAction.quit(self, SLOT("slotQuit()"), actionCollection())

        # "Edit" menu items
        @undoAction     = KDE::StdAction.undo(self, SLOT("slotUndo()"), actionCollection())
        @redoAction     = KDE::StdAction.redo(self, SLOT("slotRedo()"), actionCollection())
        @cutAction      = KDE::StdAction.cut(self, SLOT("slotCut()"), actionCollection())
        @copyAction     = KDE::StdAction.copy(self, SLOT("slotCopy()"), actionCollection())
        @pasteAction    = KDE::StdAction.paste(self, SLOT("slotPaste()"), actionCollection())
        @findAction     = KDE::StdAction.find(self, SLOT("slotFind()"), actionCollection())
        @findNextAction = KDE::StdAction.findNext(self, SLOT("slotFindNext()"), actionCollection())
        @replaceAction  = KDE::StdAction.replace(self, SLOT("slotReplace()"), actionCollection())
        
		# For actions that are not "standard", you can create your
        # own actions using KDE::Action. This example doesn't include
        # an icon, but there is a KDE::Action constructor that will
        # allow you to specify an icon (for toolbar use, for instance),
        # or you can use KDE::Action.setIcon to set/change the icon. You
        # can also add a tooltip with KDE::Action.setToolTip

        # This KAction constructor requires a string, an accelerator (0
        # in this case), a slot, and a QObject (None in this case)
		
		@specialAction  = KDE::Action.new(i18n("Special"), KDE::Shortcut.new(0), self, SLOT('slotSpecial()'), actionCollection(), "specialActionName")
	end
	
    def initMenus()
        # plug the actions into the menus

        fileMenu = Qt::PopupMenu.new(self)
        @newAction.plug(fileMenu)
        @openAction.plug(fileMenu)
        fileMenu.insertSeparator()
        @saveAction.plug(fileMenu)
        @saveAsAction.plug(fileMenu)
        fileMenu.insertSeparator()
        @printAction.plug(fileMenu)
        fileMenu.insertSeparator()
        @quitAction.plug(fileMenu)
        menuBar().insertItem(i18n("&File"), fileMenu)

        editMenu = Qt::PopupMenu.new(self)
        @undoAction.plug(editMenu)
        @redoAction.plug(editMenu)
        editMenu.insertSeparator()
        @cutAction.plug(editMenu)
        @copyAction.plug(editMenu)
        @pasteAction.plug(editMenu)
        editMenu.insertSeparator()
        @findAction.plug(editMenu)
        @findNextAction.plug(editMenu)
        @replaceAction.plug(editMenu)
        editMenu.insertSeparator()
        @specialAction.plug(editMenu)
        menuBar().insertItem(i18n("&Edit"), editMenu)

        # Uses the info from KAboutData (specified below)
        # to construct the "About" box in the Help menu

        helpMenu = helpMenu("")
        menuBar().insertItem(i18n("&Help"), helpMenu)
	end

    def initToolBar()
        # Add some (but not all) actions to the toolbar

        @newAction.plug(toolBar())
        @openAction.plug(toolBar())
        @saveAction.plug(toolBar())
        @cutAction.plug(toolBar())
        @copyAction.plug(toolBar())
        @pasteAction.plug(toolBar())
	end

    def initStatusBar()
        # Initialize the status bar

        statusBar().insertItem("", STATUSBAR_LEFT, 1000, true)
        statusBar().insertItem("", STATUSBAR_MIDDLE, 1000, true)
        statusBar().insertItem("", STATUSBAR_RIGHT, 1000, true)
	end

#-------------------- slots -----------------------------------------------

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
        notImpl("Quit")
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
