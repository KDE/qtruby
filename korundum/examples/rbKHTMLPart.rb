=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

#
#  pyParts.py (C) 2002 Jim Bublitz <jbublitz@nwinternet.com>
#

=begin

This is an extemely simple and crude example of using
a KHTMLPart - I put it together mostly to make sure
the openURL method worked correctly after some modifications
done in KParts::ReadOnlyPart. It took exactly four lines
added to a basic Korundum app framework to display a URL
via the 'net:

    w  = KDE::HTMLPart.new(self, "HTMLPart", self)
    w.openURL(KDE::URL.new("http://www.kde.org"))
    w.view().setGeometry(30, 55, 500, 400)
    w.show()

You can play around with the commented out lines or add
additional code to make this do something useful. The
.rc for khtnmlpart (sorry, I never looked it up), doesn't
seem to provide much help. Also, to follow links, you
probably need to connect some signals to slots. I
haven't tried it, but this should work with a plain
KMainWindow or other widget too.

The KDE website also incorporates gifs, jpegs, and
I believe CSS too. Playing around with some other
sites, it appears the font defaults could use some
improvement.

NOTE!!! For this to work, you (obviously) need to have
a route to the internet established or specify a local
URL - PyKDE/KDE will take care of everything else.

Perceptive users will notice the KHTMLPart code is
lifted from the KDE classref.

=end

require 'korundum4'

# Note that we use KParts.MainWindow, not KMainWindow as the superclass
# (KParts.MainWindow subclasses KMainWindow). Also, be sure the 'apply'
# clause references KParts.MainWindow - it's a hard bug to track down
# if it doesn't.

class RbPartsMW < KParts::MainWindow
	slots 'close()', 'optionsShowToolbar()', 'optionsShowStatusbar()', 'optionsConfigureKeys()',
			'optionsConfigureToolbars()'
			
	TOOLBAR_EXIT = 0
	TOOLBAR_OPEN = 1
        
		def initialize(*k)
                super

                # Create the actions for our menu/toolbar to use
                # Keep in mind that the part loaded will provide its
                # own menu/toolbar entries

                # check out KParts.MainWindow's ancestry to see where
                # some of this and later stuff (like self.actionCollection () )
                # comes from

                quitAction    = KDE::StdAction.quit(self, SLOT('close()'), actionCollection())

				createStandardStatusBarAction()
#                @m_toolbarAction = KDE::StdAction.showToolbar(self, SLOT('optionsShowToolbar()'), actionCollection())
                @m_statusbarAction = KDE::StdAction.showStatusbar(self, SLOT('optionsShowStatusbar()'), actionCollection())

                KDE::StdAction.keyBindings(self, SLOT('optionsConfigureKeys()'), actionCollection())
                KDE::StdAction.configureToolbars(self, SLOT('optionsConfigureToolbars()'), actionCollection())

                path = Dir.getwd() + '/'
                setGeometry(0, 0, 600, 500)

                # point to our XML file
                setXMLFile(path + "rbParts.rc", false)

                # The next few lines are all that's necessary to
                # create a web browser (of course you have to edit
                # this file to change url's)

                w   = KDE::HTMLPart.new(self, "HTMLPart", self)
                w.openURL(KDE::URL.new("http://www.kde.org"))

                w.view().setGeometry(30, 55, 500, 400)


#                self.v = KHTMLView (self.w, self)

#                self.setCentralWidget (self.v)

#                self.createGUI (self.w)

                w.show()
		end




        # slots for our actions
        def optionsShowToolbar()
                if @m_toolbarAction.isChecked()
                        toolBar().show()
                else
                        toolBar().hide()
				end
		end

        def optionsShowStatusbar()
                if @m_statusbarAction.isChecked()
                        statusBar().show()
                else
                        statusBar().hide()
				end
		end


        def optionsConfigureKeys()
                KDE::KeyDialog.configureActionKeys(actionCollection(), xmlFile())
		end


        def optionsConfigureToolbars()
                dlg = KDE::EditToolbar.new(actionCollection(), xmlFile())
                if dlg.exec()
                        createGUI(self)
				end
		end


        # some boilerplate left over from pyKLess/KLess
        def queryClose()
                res = KDE::MessageBox.warningYesNoCancel(self,
                        i18n("Save changes to Document?<br>(Does not make sense, we know, but it is just a programming example :-)"))
                if res == KDE::MessageBox::Yes
	        #// save document here. If saving fails, return FALSE
	                return true

                elsif res == KDE::MessageBox::No
                        return true

                else #// cancel
	                return false
				end
		end

        def queryExit()
                #// this slot is invoked in addition when the *last* window is going
                #// to be closed. We could do some final cleanup here.
                return true #// accept
		end

        # I'm not sure the session mgmt stuff here works

        # Session management: save data
        def saveProperties(config)
        # This is provided just as an example.
        # It is generally not so good to save the raw contents of an application
        # in its configuration file (as this example does).
        # It is preferable to save the contents in a file on the application's
        # data zone and save an URL to it in the configuration resource.
                config.writeEntry("text", edit.text())
		end


        # Session management: read data again
        def readProperties(config)
        # See above
                edit.setText(config.readEntry("text"))
		end
end


#------------- main ----------------------------

# A Human readable description of your program
description = "KHTMLPart - simple example"
# The version
version = "0.1"

# stuff for the "About" menu
aboutData = KDE::AboutData.new("rbKHTMLPart", "rbHTMLPart",
    version, description, KDE::AboutData::License_GPL,
    "(c) 2002, Jim Bublitz")

aboutData.addAuthor("Jim Bublitz", "Example for PyKDE", "jbublitz@nwinternet.com")
aboutData.addAuthor('Richard Dale', 'Example for Korundum', 'Richard_Dale@tipitina.demon.co.uk')

# This MUST go here (before KApplication () is called)
KDE::CmdLineArgs.init(ARGV, aboutData)

app = KDE::Application.new()

if app.isRestored()
        KDE::MainWindow.kRestoreMainWindows(RbPartsMW)
else
        # no session management: just create one window
        # this is our KParts::MainWindow derived class
        parts = RbPartsMW.new(nil, "rbParts")
        if ARGV.length > 1
        # read kcmdlineargs.h for the full unabridged instructions
        # on using KCmdLineArgs, it's pretty confusing at first, but it works
        # This is pretty useless in this program - you might want to
        # expand this in your app (to load a file, etc)
                args = KDE::CmdLineArgs.parsedArgs()
		end
end

parts.show()
app.exec()
