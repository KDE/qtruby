require 'Korundum'
    
class Browser < KDE::MainWindow
    k_dcop  'void setURL(QString)'
	
	slots   'fileSetDefaultPage()', 
            'changeLocation()',
            'bookLocation()',
            'gotoPreviousPage()',
            'openURLRequest(const KURL&, const KParts::URLArgs&)'
 
    def initialize( name )
        super(nil, name)
        @history = []

        KDE::StdAction.quit(self, SLOT('close()'), actionCollection())

        KDE::Action.new(i18n("&Set default page"), "gohome", KDE::Shortcut.new(0), self,
		    SLOT('fileSetDefaultPage()'), actionCollection(), "set_default_page")

        KDE::Action.new(i18n("Add to Bookmarks"), "reload", KDE::Shortcut.new(0), self,
		    SLOT('bookLocation()'), actionCollection(), "add_to_bookmarks")

        KDE::Action.new(i18n("Back to previous page"), "back", KDE::Shortcut.new(0), self,
		    SLOT('gotoPreviousPage()'), actionCollection(), "back")

        actionCollection().action("back").setEnabled(false)

        createGUI(Dir.getwd + "/p9ui.rc")

        vbox = Qt::VBox.new( self )
 
        @location = Qt::LineEdit.new( vbox )

        config = $kapp.config()
        config.setGroup("Settings")
        @location.text = config.readEntry( "defaultPage", "http://localhost")

        connect( @location , SIGNAL( 'returnPressed()' ),
                    self, SLOT( 'changeLocation()' ) )

		@browser = KDE::HTMLPart.new( vbox )
        @browser.openURL( KDE::URL.new(@location.text()) )

        connect( @browser.browserExtension(),
	            SIGNAL( 'openURLRequest( const KURL&, const KParts::URLArgs& )' ),
	            self, SLOT( 'openURLRequest(const KURL&, const KParts::URLArgs& )' ) )           	     
        setCentralWidget(vbox)
    end


    def changeLocation()
        @history.push( @browser.url().url() );
        actionCollection().action("back").setEnabled(true)
        @browser.openURL( KDE::URL.new(@location.text()) )
    end

    def setURL( url )
        @location.text = url
        changeLocation()
    end

    def openURLRequest(url, part)
        setURL( url.url() )
    end

    def gotoPreviousPage()
        @location.text = @history.pop() 
        if @history.empty?
            actionCollection().action("back").setEnabled(false)
        end
        @browser.openURL( KDE::URL.new(@location.text()) )
    end

    def bookLocation()
        dcopRef = KDE::DCOPRef.new("p8", "BookMarkList")
        if ! dcopRef.add(@location.text())
            Qt.qWarning("Error with DCOP\n")
        end
    end

    def fileSetDefaultPage()
        config = $kapp.config()
 
        config.group = "Settings"
        config.writeEntry( "defaultPage", @browser.url().url() )
    end
end

	aboutdata = KDE::AboutData.new("p9", "Tutorial - p9",
      "1.0", "Step 9 of a simple tutorial", KDE::AboutData::License_GPL,
      "(C) 2000, 2001 Antonio Larrosa Jimenez","",
      "http://devel-home.kde.org/~larrosa/tutorial.html")
    aboutdata.addAuthor("Antonio Larrosa Jimenez",
      "Original Developer/Mantainer","larrosa@kde.org",
      "http://devel-home.kde.org/~larrosa/index.html")
    
	KDE::CmdLineArgs.init(ARGV, aboutdata)
	
    a = KDE::UniqueApplication.new()
	
    window = Browser.new( "Tutorial - p9" )
    window.resize( 300, 200 )
	
    a.mainWidget = window
    window.show
	
    a.exec 
