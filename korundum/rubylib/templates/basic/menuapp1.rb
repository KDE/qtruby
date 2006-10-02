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
        fileMenu = Qt::PopupMenu.new(self)
        
		# "File" menu items
        fileMenu.insertItem(i18n("New"), self, SLOT('slotNew()'), Qt::KeySequence.new(KDE::StdAccel.openNew().keyCodeQt()))
        fileMenu.insertItem(i18n("Open"), self, SLOT('slotOpen()'), Qt::KeySequence.new(KDE::StdAccel.open().keyCodeQt()))
        fileMenu.insertSeparator()
        fileMenu.insertItem(i18n("Save"), self, SLOT('slotSave()'), Qt::KeySequence.new(KDE::StdAccel.save().keyCodeQt()))
        fileMenu.insertItem(i18n("SaveAs"), self, SLOT('slotSaveAs()'))
        fileMenu.insertSeparator()
        fileMenu.insertItem(i18n("Print"), self, SLOT('slotPrint()'), Qt::KeySequence.new(KDE::StdAccel.print().keyCodeQt()))
        fileMenu.insertSeparator()
        fileMenu.insertItem(i18n("&Quit"), self, SLOT('slotQuit()'), Qt::KeySequence.new(KDE::StdAccel.quit().keyCodeQt()))

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

        menuBar().insertItem(i18n("&Edit"), editMenu)

        helpMenu = helpMenu("")
        menuBar().insertItem(i18n("&Help"), helpMenu)
	end

    def initToolBar()
        icons = KDE::IconLoader.new()

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
