require 'Korundum'
 
class Browser < KDE::MainWindow
    k_dcop  'void setURL(QString)'
	
	slots   'fileSetDefaultPage()', 
            'changeLocation()',
            'bookLocation()',
            'gotoPreviousPage()',
            'openURLRequest(const KURL&, const KParts::URLArgs&)'
 
    TOOLBAR_ID_ADDBOOKMARK = 1
    TOOLBAR_ID_BACK = 2
    TOOLBAR_ID_QUIT = 3
 
    def initialize( name )
        super(nil, name)
        setCaption("KDE Tutorial - p7")
        @history = []

        filemenu = Qt::PopupMenu.new
        filemenu.insertItem( i18n( "&Set default page" ), 
				    self, SLOT( 'fileSetDefaultPage()' ) )
        filemenu.insertItem(i18n( "&Quit" ), $kapp, SLOT( 'quit()' ))
        about =
                i18n("p7 1.0\n\n" +
                     "(C) 1999-2002 Antonio Larrosa Jimenez\n" +
                     "larrosa@kde.org\t\tantlarr@supercable.es\n" +
                     "Malaga (Spain)\n\n" +
                     "Simple KDE Tutorial\n" +
                     "This tutorial comes with ABSOLUTELY NO WARRANTY \n" +
                     "This is free software, and you are welcome to redistribute it\n" +
                     "under certain conditions\n");
 
        helpmenu = helpMenu(about)
        menu = menuBar()
        menu.insertItem( i18n( "&File" ), filemenu)
        menu.insertSeparator()
        menu.insertItem(i18n("&Help"), helpmenu)
 
        toolbar = KDE::ToolBar.new(self)

        icons = KDE::IconLoader.new()
        toolbar.insertButton(icons.loadIcon("reload", KDE::Icon::Toolbar), TOOLBAR_ID_ADDBOOKMARK,
                              SIGNAL('clicked(int)'),self,SLOT('bookLocation()'),true,
                              i18n("Add to Bookmarks"))
        toolbar.insertButton(icons.loadIcon("back", KDE::Icon::Toolbar), TOOLBAR_ID_BACK,
                              SIGNAL('clicked(int)'),self,SLOT('gotoPreviousPage()'),
                              false, i18n("Back to previous page"))
        toolbar.insertButton(icons.loadIcon("exit", KDE::Icon::Toolbar), TOOLBAR_ID_QUIT,
                              SIGNAL('clicked(int)'),$kapp,SLOT('quit()'),true,
                              i18n("Quit the application"))
        addToolBar(toolbar)
 
        vbox = Qt::VBox.new( self )
 
        @location = Qt::LineEdit.new( vbox )
 
        config = $kapp.config()
        config.setGroup("Settings")
        @location.text = config.readEntry( "defaultPage", "http://localhost")
 
        connect( @location , SIGNAL( 'returnPressed()' ),
                    self, SLOT( 'changeLocation()' ) )
 
        split = Qt::Splitter.new( vbox )
        split.setOpaqueResize()
        
		@browser = KDE::HTMLPart.new( split )
        @browser.openURL( KDE::URL.new(@location.text()) )
 
        connect( @browser.browserExtension(),
	            SIGNAL( 'openURLRequest( const KURL&, const KParts::URLArgs& )' ),
	            self, SLOT( 'openURLRequest(const KURL&, const KParts::URLArgs& )' ) )           	     
        setCentralWidget(vbox)
    end
  
    def changeLocation()
        @history.push( @browser.url().url() );
        toolBar().setItemEnabled( TOOLBAR_ID_BACK, true)
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
            toolBar().setItemEnabled(TOOLBAR_ID_BACK, false)
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

    about = KDE::AboutData.new("p7", "Tutorial - p7", "0.1")
    KDE::CmdLineArgs.init(ARGV, about)
    a = KDE::UniqueApplication.new()
	
    window = Browser.new( "Tutorial - p7" )
    window.resize( 300, 200 )
	
    a.mainWidget = window
    window.show
	
    a.exec 

