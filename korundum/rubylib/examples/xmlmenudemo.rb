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
of this software and associated documentation files(the "Software"), to
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

class MainWin < KDE::MainWindow

	STATUSBAR_LEFT   = 1
	STATUSBAR_MIDDLE = 2
	STATUSBAR_RIGHT  = 3

	slots	'slotFake()', 'slotNew()', 'slotOpen()', 'slotSave()', 'slotSaveAs()', 'slotPrint()', 'slotRadio()',
			'slotQuit()', 'slotUndo()', 'slotRedo()', 'slotCut()', 'slotCopy()', 'slotPaste()', 'slotFind()',
			'slotFindNext()', 'slotReplace()', 'slotSpecial()', 'slotToggle2()', 'slotZoomIn()', 'slotZoomOut()'

    def initialize(*args)
        super

        initActions()
        setGeometry(0, 0, 350, 200)

        # The second arg of createGUI needs to be 0(or false)
        # to enable XMLGUI features like ActionList(in 'dynamicActions')
        # If the default is used(true), the dynamic actions will not
        # appear in the menus
		uifilebase = Dir.getwd + '/xmlmenudemoui.rc'
        createGUI(uifilebase, false)

        dynamicActions()

        # Can't do this until the toolBar has been created in createGUI
        stretchlbl = Qt::Label.new("", toolBar())
        toolBar().setStretchableWidget(stretchlbl)

        initStatusBar()

        @saveAction.setEnabled(false)
        @saveAsAction.setEnabled(false)
	end

    def initActions()
        # This is used in all of the KDE::Action/KDE::StdAction constructors --
        # Seems more efficient to only do the call once
        acts = actionCollection()

        # "File" menu items
        newAction      = KDE::StdAction.openNew(self, SLOT('slotNew()'), acts)
        openAction     = KDE::StdAction.open(self, SLOT('slotOpen()'), acts)
        @saveAction     = KDE::StdAction.save(self, SLOT('slotSave()'), acts)
        @saveAsAction   = KDE::StdAction.saveAs(self, SLOT('slotSaveAs()'), acts)
        printAction    = KDE::StdAction.print(self, SLOT('slotPrint()'), acts)
        quitAction     = KDE::StdAction.quit(self, SLOT('slotQuit()'), acts)

        # "Edit" menu items
        undoAction     = KDE::StdAction.undo(self, SLOT('slotUndo()'), acts)
        redoAction     = KDE::StdAction.redo(self, SLOT('slotRedo()'), acts)
        cutAction      = KDE::StdAction.cut(self, SLOT('slotCut()'), acts)
        copyAction     = KDE::StdAction.copy(self, SLOT('slotCopy()'), acts)
        pasteAction    = KDE::StdAction.paste(self, SLOT('slotPaste()'), acts)
        findAction     = KDE::StdAction.find(self, SLOT('slotFind()'), acts)
        findNextAction = KDE::StdAction.findNext(self, SLOT('slotFindNext()'), acts)
        replaceAction  = KDE::StdAction.replace(self, SLOT('slotReplace()'), acts)

        # NOTE!!!! You must specify a parent and name for the action object in its constructor
        # Normally in a constructor like
        #
        #   someObject(Qt::Widget *parent = 0, const char *name = 0)
        #
        # the parent may or may not be assigned, but widgets usually ignore the
        # name argument. For an action of *any* type(other than KDE::StdAction),
        # the 'name' argument is what is used to load the action into the menus
        # and toolBar(in the line below, "specialActionName"). The XMLGUI mechanism
        # has no way to find out about the action objects except through their
        # object names - the variable the object is assigned to('specialAction')
        # has no meaning in XNLGUI terms except through the objects 'name' member value

        specialAction  = KDE::Action.new(i18n("Special"), KDE::Shortcut.new(0), self, SLOT('slotSpecial()'), acts, "specialActionName")

        # Demo menu items

        # KDE::ToggleAction has an isChecked member and emits the "toggle" signal
        toggle1Action  = KDE::ToggleAction.new("Toggle 1", KDE::Shortcut.new(0), acts, "toggle1Action")
        toggle2Action  = KDE::ToggleAction.new("Toggle 2", KDE::Shortcut.new(0), self, SLOT('slotToggle2()'), acts, "toggle2Action")

        # A separator - create once/use everywhere
        separateAction = KDE::ActionSeparator.new(acts, "separateAction")

        # Font stuff in menus or toolbar
        fontAction     = KDE::FontAction.new("Font", KDE::Shortcut.new(0), acts, "fontAction")
        fontSizeAction = KDE::FontSizeAction.new("Font Size", KDE::Shortcut.new(0), acts, "fontSizeAction")

        fontAction.setComboWidth(150)
        fontSizeAction.setComboWidth(75)

       # Need to assign an icon to actionMenu below
        icons = KDE::IconLoader.new()
        iconSet = Qt::IconSet.new(icons.loadIcon("viewmag", KDE::Icon::Toolbar))

        # Nested menus using KDE::Actions.new(also nested on toolbar)
        actionMenu     = KDE::ActionMenu.new("Action Menu", acts, "actionMenu")
        actionMenu.setIconSet(iconSet)

        # By using KDE::StdAction here, the XMLGUI mechanism will automatically
        # create a 'View' menu and insert "Zoom In" and "Zoom Out" objects
        # in it. This happens because before parsing our *ui.rc file,
        # the standard KDE::DE file config/ui/ui_standards.rc is parsed, and
        # then our *ui.rc file is merged with the result - this gives KDE::DE
        # menus and toolBars a standard "look" and item placement(including
        # separators). Creating the KDE::StdActions alone is sufficient - you
        # could delete their references from the *ui.rc file and the menu
        # items would still be created via ui_standards.rc
        actionMenu.insert(KDE::StdAction.zoomIn(self, SLOT('slotZoomIn()'), acts))
        actionMenu.insert(KDE::StdAction.zoomOut(self, SLOT('slotZoomOut()'), acts))

        radio1Action = KDE::RadioAction.new("Radio 1", KDE::Shortcut.new(0), self, SLOT('slotRadio()'), acts, "radio1")
        radio1Action.setExclusiveGroup("Radio")
        radio1Action.setChecked(1)
        radio2Action = KDE::RadioAction.new("Radio 2", KDE::Shortcut.new(0), self, SLOT('slotRadio()'), acts, "radio2")
        radio2Action.setExclusiveGroup("Radio")
        radio3Action = KDE::RadioAction.new("Radio 3", KDE::Shortcut.new(0), self, SLOT('slotRadio()'), acts, "radio3")
        radio3Action.setExclusiveGroup("Radio")
	end


    def initStatusBar()
        statusBar().insertItem("", STATUSBAR_LEFT, 1000, true)
        statusBar().insertItem("", STATUSBAR_MIDDLE, 1000, true)
        statusBar().insertItem("", STATUSBAR_RIGHT, 1000, true)
	end

    def dynamicActions()
        # This creates something like a 'recent files list' in the 'File' menu
        # (There is a KDE::RecentFilesAction that probably should be used instead,
        # but this demos the use of action lists)
        # The code here corresponds to the <ActionList name="recent"/> entry
        # in the rc file

        # Just fake some filenames for now
        fakeFiles = ["kaction.sip", "kxmlguiclient.sip"]

        # Clear the old entries, so we don't end up accumulating entries in the menu
        unplugActionList("recent");
        dynamicActionsList = []

        # Create a KDE::Action for each entry and store the KDE::Actions in a list
        # Use 'nil' for the KDE::ActionCollection argument in the KDE::Action constructor
        # in this case only
        (0...fakeFiles.length).each do |i|
            act = KDE::Action.new(i18n(" &#{i.to_s} #{fakeFiles[i]}"), KDE::Shortcut.new(0),
                                 self, SLOT('slotFake()'), nil, fakeFiles[i].sub(/.sip$/,'') + "open")
            dynamicActionsList << act
		end

        # Update the menu with the most recent KDE::Actions
        plugActionList("recent", dynamicActionsList)
	end


#-------------------- slots -----------------------------------------------

    def slotFake()
        # sender() should be called before anything else
        # (including "notImpl") so the correct sender
        # value is returned
        sender = sender().name()
        notImpl("Recent files(#{sender})")
	end

    # 'id' is for toolbar button signals - ignored for menu signals
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

    def slotRadio()
        sender = sender().name()
        notImpl("Radio #{sender}")
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

# The appName(xmlmenudemo - first argument) is required
# if the program is to automatically locate it *ui.rc file
aboutData   = KDE::AboutData.new("xmlmenudemo", "xmlmenudemo",
    version, description, KDE::AboutData::License_GPL,
    "(C) 2003 whoever the author is")

aboutData.addAuthor("author1", "whatever they did", "email@somedomain")
aboutData.addAuthor("author2", "they did something else", "another@email.address")

# mainpath = os.path.dirname(os.path.abspath(sys.argv[0]))
KDE::CmdLineArgs.init(ARGV, aboutData)

KDE::CmdLineArgs.addCmdLineOptions([["+files", "File to open", ""]])

app = KDE::Application.new()
mainWindow = MainWin.new(nil, "main window")
mainWindow.show
app.exec
