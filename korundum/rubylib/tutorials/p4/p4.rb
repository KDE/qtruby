require 'Korundum'
 
class MainWindow < KDE::MainWindow
    slots   'changeLocation()', 
            'openURLRequest(const KURL &, const KParts::URLArgs & )'
 
    def initialize( name )
        super(nil, name)
        setCaption("KDE Tutorial - p4")

        filemenu = Qt::PopupMenu.new
        filemenu.insertItem( i18n( "&Quit" ), $kapp, SLOT( 'quit()' ) )
        about =
                i18n("p4 1.0\n\n" +
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
 
        vbox = Qt::VBox.new( self )
 
        @location = Qt::LineEdit.new( vbox )
        @location.setText( "http://localhost" )
 
        @browser = KDE::HTMLPart.new( vbox )
        @browser.openURL( KDE::URL.new(@location.text()) )
 
        connect( @location , SIGNAL( 'returnPressed()' ),
                    self, SLOT( 'changeLocation()' ) )
 
        connect( @browser.browserExtension(),
                    SIGNAL( 'openURLRequest( const KURL &, const KParts::URLArgs & )' ),
                    self, SLOT( 'openURLRequest(const KURL &, const KParts::URLArgs & )' ) )

        setCentralWidget(vbox)
    end                                                                               

    def changeLocation()
        @browser.openURL( KDE::URL.new(@location.text()) )
    end

    def openURLRequest(url, part)
        @location.text = url.url()
        changeLocation()
    end
end

    about = KDE::AboutData.new("p4", "Tutorial - p4", "0.1")
    KDE::CmdLineArgs.init(ARGV, about)
    a = KDE::Application.new()
        
    window = MainWindow.new( "Tutorial - p4" )
    window.resize( 300, 200 )
        
    a.mainWidget = window
    window.show
        
    a.exec