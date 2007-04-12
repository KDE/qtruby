=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

=begin
This template constructs an application with menus, toolbar and statusbar.
It uses KDE classes and methods that simplify the task of building and
operating a GUI. It is recommended that this approach be used, rather
than the primitive approach in menuapp1.py
=end

=begin
Copyright 2003 Jim Bublitz

Terms and Conditions

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
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
include KDE

class MainWin < MainWindow
	
	slots	'slotNew()', 'slotOpen()', 'slotSave()', 'slotSaveAs()', 'slotPrint()', 'slotQuit()', 'slotUndo()',
 			'slotRedo()', 'slotCut()', 'slotCopy()', 'slotPaste()', 'slotFind()', 'slotFindNext()', 'slotReplace()',
			'slotSpecial()', 'slotToggle2()', 'slotZoomIn()', 'slotZoomOut()'
	
	STATUSBAR_LEFT   = 1
	STATUSBAR_MIDDLE = 2
	STATUSBAR_RIGHT  = 3
	
    def initialize(*args)
        super

        initActions()
        initMenus()
        initToolBar()
        initStatusBar()

        @saveAction.setEnabled(false)
        @saveAsAction.setEnabled(false)
	end
	
    def initActions()
        # "File" menu items
        @newAction    = StdAction.openNew(self, SLOT('slotNew()'), actionCollection())
        @openAction   = StdAction.open(self, SLOT('slotOpen()'), actionCollection())
        @saveAction   = StdAction.save(self, SLOT('slotSave()'), actionCollection())
        @saveAsAction = StdAction.saveAs(self, SLOT('slotSaveAs()'), actionCollection())
        @printAction  = StdAction.print(self, SLOT('slotPrint()'), actionCollection())
        @quitAction   = StdAction.quit(self, SLOT('slotQuit()'), actionCollection())

        # "Edit" menu items
        @undoAction     = StdAction.undo(self, SLOT('slotUndo()'), actionCollection())
        @redoAction     = StdAction.redo(self, SLOT('slotRedo()'), actionCollection())
        @cutAction      = StdAction.cut(self, SLOT('slotCut()'), actionCollection())
        @copyAction     = StdAction.copy(self, SLOT('slotCopy()'), actionCollection())
        @pasteAction    = StdAction.paste(self, SLOT('slotPaste()'), actionCollection())
        @findAction     = StdAction.find(self, SLOT('slotFind()'), actionCollection())
        @findNextAction = StdAction.findNext(self, SLOT('slotFindNext()'), actionCollection())
        @replaceAction  = StdAction.replace(self, SLOT('slotReplace()'), actionCollection())
        @specialAction  = Action.new(i18n("Special"), Shortcut.new(), self, SLOT('slotSpecial()'), actionCollection(), "special")

        # Demo menu items

        # KToggleAction has an isChecked member and emits the "toggle" signal
        @toggle1Action  = ToggleAction.new("Toggle 1")
        @toggle2Action  = ToggleAction.new("Toggle 2", Shortcut.new(), self, SLOT('slotToggle2()'), nil)

        # A separator - create once/use everywhere
        @separateAction = ActionSeparator.new()

        # Font stuff in menus or toolbar
        @fontAction     = FontAction.new("Font")
        @fontSizeAction = FontSizeAction.new("Font Size")

        # Need to assign an icon to actionMenu below
        icons = IconLoader.new()
        iconSet = Qt::IconSet.new(icons.loadIcon("viewmag", Icon::Toolbar))

        # Nested menus using KActions (also nested on toolbar)
        @actionMenu     = ActionMenu.new("Action Menu")
        @actionMenu.setIconSet(iconSet)
        @actionMenu.insert(StdAction.zoomIn(self, SLOT('slotZoomIn()'), actionCollection()))
        @actionMenu.insert(StdAction.zoomOut(self, SLOT('slotZoomOut()'), actionCollection()))

        # Doesn't work in KDE 2.1.1
#        radio1Action   = KRadioAction ("Radio 1")
#        radio1Action.setExclusiveGroup ("Radio")
#        radio2Action   = KRadioAction ("Radio 2")
#        radio2Action.setExclusiveGroup ("Radio")
#        radio3Action   = KRadioAction ("Radio 3")
#        radio3Action.setExclusiveGroup ("Radio")
	end

    def initMenus()
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

        demoMenu = Qt::PopupMenu.new(self)
        @toggle1Action.plug(demoMenu)
        @toggle2Action.plug(demoMenu)
        @separateAction.plug(demoMenu)
        @fontAction.plug(demoMenu)
        @fontSizeAction.plug(demoMenu)
        @actionMenu.plug(demoMenu)
#        radio1Action.plug(demoMenu)
#        radio2Action.plug(demoMenu)
#        radio3Action.plug(demoMenu)
        menuBar().insertItem(i18n("&Demo"), demoMenu)

        # This really belongs in Kicker, not here,
        # but it actually works
        # wlMenu = WindowListMenu.new(self)
        # wlMenu.init()
        # menuBar().insertItem(i18n("&WindowListMenu"), wlMenu)

        helpMenu = helpMenu("")
        menuBar().insertItem(i18n("&Help"), helpMenu)
	end

    def initToolBar()
        @newAction.plug(toolBar())
        @openAction.plug(toolBar())
        @saveAction.plug(toolBar())
        @cutAction.plug(toolBar())
        @copyAction.plug(toolBar())
        @pasteAction.plug(toolBar())

        @separateAction.plug(toolBar())
        @separateAction.plug(toolBar())
        @separateAction.plug(toolBar())

        @fontAction.plug(toolBar())
        @separateAction.plug(toolBar())
        @fontAction.setComboWidth(150)

        @fontSizeAction.plug(toolBar())
        @fontSizeAction.setComboWidth(75)

        @separateAction.plug(toolBar())

        # This works, but you have to hold down the
        # button in the toolbar and wait a bit
        @actionMenu.plug(toolBar())
        # This appears to do nothing
        @actionMenu.setDelayed(false)

       # Need this to keep the font comboboxes from stretching
        # to the full width of the toolbar when the window is
        # maximized (comment out the next two lines to see
        # what happens)
        stretchlbl = Qt::Label.new("", toolBar())
        toolBar().setStretchableWidget(stretchlbl)

#        toolBar().setHorizontalStretchable(false)
	end

    def initStatusBar()
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

    def slotCut (id = -1)
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

    def slotSpecial()
        notImpl("Special")
	end

    def slotToggle2()
        notImpl("Toggle")
	end

    def slotZoomIn()
        notImpl("Zoom In")
	end

    def slotZoomOut()
        notImpl("Zoom Out")
	end

    def notImpl(item)
        statusBar().changeItem("#{item} not implemented", STATUSBAR_LEFT)
        MessageBox.error(self, "#{item} not implemented", "Not Implemented")
        statusBar().changeItem("", STATUSBAR_LEFT)
	end
end

#-------------------- main ------------------------------------------------

description = "A basic application template"
version     = "1.0"
aboutData   = AboutData.new("menudemo", "MenuDemo",
    version, description, AboutData::License_GPL,
    "(C) 2003 whoever the author is")

aboutData.addAuthor("author1", "whatever they did", "email@somedomain")
aboutData.addAuthor("author2", "they did something else", "another@email.address")

CmdLineArgs.init(ARGV, aboutData)

CmdLineArgs.addCmdLineOptions([["+files", "File to open", ""]])

app = Application.new()
mainWindow = MainWin.new(nil, "main window")
mainWindow.show()
app.exec()
