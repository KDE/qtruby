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

        # Create actions that correspond to those in the XML file
        initActions()
		
        # Parse the default XML file (<appName>ui.rc> and create
        # the menus and toolbar. This single line (and the XML
        # file it reads) replace initMenus and initToolBar from
        # menuapp2.rb. Otherwise, the menuapp2 and menuapp3
        # are identical  'createGUI' expects to find 'menuapp3ui.rc'
        # either in the directory menuapp3.rb is run from, or
        # in $KDEDIR/apps/menuapp3/
        createGUI()

        # Create the status bar
        initStatusBar()

        # Disable a couple of menu items using their actions
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

        # Because the XMLGUI mechanism parses $KDEDIR/config/ui/ui_standards.rc
        # before parsing and merging menuapp3ui.rc, it actually isn't
        # necessary to list KDE::StdAction actions in menuapp3.rc. THE XMLGUI
        # code will create menu/toolbar items and place them *automatically*
        # if you defined the KDE::StdActions as below. In fact, you can't override
        # this behavior using KDE::StdActions - if you want menus to be "non-standard"
        # KDE menus (eg 'Cut' in the 'File' menu), you'll need to create your
        # actions from KDE::Action instead of KDE::StdAction. Obviously it makes more
        # sense to use the mechanism provided to produce consistent menus and
        # toolbars. You can "unplug" items if, for example, you don't want them
        # in the toolBar.
        
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
        
		# For ANYTHING constructed from KDE::Action or its descendants (KDE::ActionMenu, KDE::ActionSeparator,
        # KDE::FontAction, etc) you MUST provide the actionCollection () parent and an object
        # name ("specialActionName") or the XMLGUI mechanism will not be able to locate the
        # action. XMLGUI finds the action via its member name value, NOT via its variable name.
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

# To use the XMLGUI mechanism, you MUST provide an appName
# (the first argument to KDE::AboutData below) - the XML spec
# for the interface will be in <appName>ui.rc (don't forget
# the "ui" suffix to the application name)
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

