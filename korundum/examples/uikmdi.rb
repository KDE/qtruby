
=begin
This is a ruby version of Jim Bublitz's python original. The ruby behaviour
is 'crash for crash' identical - so the problems described below are 
related to KMDI, and not the bindings.
=end

=begin
A rough Python translation of the ideas presented in this KMDI tutorial:

    http://web.tiscali.it/andreabergia/kmditutorial.html

What does work:

    IDEAlMode - yay!

    Adding and closing child views

    Two-way syncing between a tool widget and a matching child view

All is not rosy, however:

    Instances of the KmdiExample maintain a dictionary of child views.  Values
    cannot be deleted from this dictionary during a window close (causes an
    immediate segfault).

    Child views created after initialization aren't numbered correctly; given
    the first problem, it's harder to do this than it's really worth.

    The example segfaults at shutdown if the tool (on the left) is is open but
    is not in overlap-mode.

=end

require 'Korundum'
include KDE

class KmdiExample < KDE::MdiMainFrm

	slots	'closeChild(KMdiChildView*)',
			'syncFromChildView(KMdiChildView*)',
			'syncFromMainTool(QListBoxItem*)',
			'activatedMessage(KMdiChildView*)',
			'newView()', 'close()', 'closeActiveChild()'
			
	def getIcon(name, group=Icon::NoGroup, size=Icon::SizeSmall)
    	# returns a kde icon by name
    	return Global.instance().iconLoader().loadIcon(name, group, size)
	end

    def initialize(parent=nil)
        super(parent, 'KmdiExample', Mdi::IDEAlMode)

    	@viewIcons = ['network', 'email', 'stop', 'back', 'forward']
    	@toolIcons = ['view_icon', 'configure']
	
        openNewAction = StdAction.openNew(self, SLOT('newView()'), actionCollection())
        quitAction = StdAction.quit(self, SLOT('close()'), actionCollection())
        closeAction = StdAction.close(self, SLOT('closeActiveChild()'), actionCollection())
    	
		uifilebase = Dir.getwd + '/uikmdi.rc'
        createGUI(uifilebase)
		# The task bar is created in the KMdiMainFrm constructor
		# and then deleted in the createGUI() call above..
		# So recreate it again to avoid a crash.
		createTaskBar() 
        statusBar()
        resize(400, 300)

        @tools = {}
		@toolIcons.each_index do |idx|
			ico = @toolIcons[idx]
			wid = KDE::ListBox.new(self, "list#{idx.to_s}")
			makeTool(wid, "Tool #{idx.to_s}", ico)
		end
        ## smells
        @mainToolWidget = @maintool = @tools['Tool 0'][0]

        @childs = {}
		@viewIcons.each_index do |idx|
			ico = @viewIcons[idx]
            makeView("View #{idx.to_s}", ico, ico)
		end

        connect(self, SIGNAL('viewActivated(KMdiChildView*)'), self, SLOT('activatedMessage(KMdiChildView*)'))
        connect(self, SIGNAL('viewActivated(KMdiChildView*)'), self, SLOT('syncFromChildView(KMdiChildView*)'))
        connect(@maintool, SIGNAL('selectionChanged(QListBoxItem*)'), self, SLOT('syncFromMainTool(QListBoxItem*)'))
        syncFromChildView(activeWindow())
	end

    def syncFromMainTool(item)
        # activate the view that matches the item text
         activateView(findWindow(item.text()))
	end

    def syncFromChildView(child)
        # sync the main tool to the indicated child
       @maintool = @mainToolWidget
		if child.nil?
			return
		end
        item = @maintool.findItem(child.tabCaption())
        if !item.nil?
            @maintool.setSelected(item, nil)
		end
	end

    def makeTool(widget, caption, icon, percent=50)
        # makes a tool from the widget
        tip = i18n("#{caption} Tool Tip")
        dock = DockWidget::DockLeft
        maindock = getMainDockWidget()
        widget.setIcon(getIcon(icon))
        tool = addToolWindow(widget, dock, maindock, percent, tip, caption)
        @tools[caption] = [widget, tool]
	end

    def makeView(label, icon, text)
        # makes a child view with a text label and a pixmap label
        view = MdiChildView.new(label, self)
        @childs[label] = view
        view.setIcon(getIcon(icon))
        layout = Qt::VBoxLayout.new(view)
        layout.setAutoAdd(true)

        lbl = Qt::Label.new(i18n("Label for a view with an icon named #{text}"), view)
        pxm = Qt::Label.new('', view)
        pxm.setPixmap(getIcon(icon, Icon::NoGroup, KDE::Icon::SizeLarge))
        addWindow(view)
        @mainToolWidget.insertItem(label)
        connect(view, SIGNAL('childWindowCloseRequest(KMdiChildView*)'), self, SLOT('closeChild(KMdiChildView*)'))
	end

    def removeMainToolItem(view)
        # remove item from the main list tool that corresponds to the view
        @maintool = @mainToolWidget
        @maintool.takeItem(@maintool.findItem(view.tabCaption(), 0))
	end

    def newView()
        # make a view when the user invokes the new action
        makeView("View ", 'network', 'A Fresh View')
#        makeView("View #{@childs.length}", 'network', 'A Fresh View')
        syncFromChildView(activeWindow())
	end

    def closeActiveChild()
        # close the current view
        removeMainToolItem(activeWindow())
        closeActiveView()
        syncFromChildView(activeWindow())
	end

    def closeChild(which)
        # called to close a view from its tab close button
        caption = which.tabCaption()
        removeMainToolItem(which)
        which.close()
        statusBar().message(i18n("#{caption} closed"))
        syncFromChildView(activeWindow())
	end

    def activatedMessage(view)
        # updates the status bar with the caption of the current view
        statusBar().message(i18n("#{view.tabCaption()} activated"))
	end
end

if $0 == __FILE__
    aname = 'uikmdi'
    desc = 'A Simple Korundum KMDI Sample'
    ver = '1.0'
    lic = AboutData::License_GPL
    author = 'Troy Melhase'
    authormail = 'troy@gci.net'

    about = AboutData.new(aname, aname, ver, desc, lic, "#{authormail} (c) 2004")
    about.addAuthor(author, 'hi, mom!', authormail)
    about.addAuthor('Jim Bublitz', 'For PyKDE', 'jbublitz@nwinternet.com')
    about.addAuthor('Richard Dale', 'For Korundum', 'Richard_Dale@tipitina.demon.co.uk')
    CmdLineArgs.init(ARGV, about)
    app = KDE::Application.new
    mainWindow = KmdiExample.new
    mainWindow.show
    app.exec
end

