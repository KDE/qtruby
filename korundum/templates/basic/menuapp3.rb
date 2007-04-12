=begin
This template constructs an application with menus, toolbar and statusbar.
It uses an XML file(menuapp3ui.rc) to specify the menu layout; all menu
items have a corresponding action defined, but no menus are created
explicitly in code. This app has the same menu layout as menuapp2.py
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

        initActions()
        createGUI()
        initStatusBar()

        @saveAction.setEnabled(false)
        @saveAsAction.setEnabled(false)
	end

    def initActions()
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
		@specialAction  = KDE::Action.new(i18n("Special"), KDE::Shortcut.new(0), self, SLOT('slotSpecial()'), actionCollection(), "specialActionName")
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
aboutData   = KDE::AboutData.new("menuapp3", "",
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

