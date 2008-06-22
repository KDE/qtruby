require 'korundum4'
require 'khtml'
 
class Browser < KDE::XmlGuiWindow
    slots   :fileSetDefaultPage, 
            :changeLocation,
            :bookLocation,
            :gotoPreviousPage,
            'openUrlRequest(KUrl, KParts::OpenUrlArguments)',
            'setUrl(QString)'

    def initialize(name)
        super(nil)
        self.objectName = name
        self.caption = "KDE Tutorial - p9"
        @history = []

        filemenu = Qt::Menu.new(i18n("&File"))

        setDefaultPageAction = KDE::Action.new(self)
        setDefaultPageAction.text = i18n("&Set default page")
        actionCollection().addAction("set_default_page", setDefaultPageAction)
        connect(    setDefaultPageAction, SIGNAL('triggered(bool)'), 
                    self, SLOT(:fileSetDefaultPage) );

        @addBookmarkAction = KDE::StandardAction.addBookmark(self, SLOT(:bookLocation), actionCollection())
        @backAction = KDE::StandardAction.back(self, SLOT(:gotoPreviousPage), actionCollection())
        @backAction.enabled = false
        @quitAction = KDE::StandardAction.quit($kapp, SLOT(:quit), actionCollection())

        about = i18n("p7 1.0\n\n" +
                     "(C) 1999-2002 Antonio Larrosa Jimenez\n" +
                     "larrosa@kde.org\t\tantlarr@supercable.es\n" +
                     "Malaga (Spain)\n\n" +
                     "Simple KDE Tutorial\n" +
                     "This tutorial comes with ABSOLUTELY NO WARRANTY \n" +
                     "This is free software, and you are welcome to redistribute it\n" +
                     "under certain conditions\n");
 
        helpmenu = helpMenu(about)
        menu = menuBar()
        menu.addMenu(filemenu)
        menu.addSeparator()
        menu.addMenu(helpmenu)
 
        toolBar.addAction(@quitAction)
        toolBar.addAction(@addBookmarkAction)
        toolBar.addAction(@backAction)
        setStandardToolBarMenuEnabled(true)

        @location = Qt::LineEdit.new
        @location.text = "http://localhost"
 
        @config = KDE::ConfigGroup.new(KDE::Global.config, "Settings")
        @location.text = @config.readEntry("defaultPage", "http://localhost")

        connect( @location , SIGNAL( 'returnPressed()' ),
                    self, SLOT( 'changeLocation()' ) )
 
        split = Qt::Splitter.new
        split.setOpaqueResize()

        widget = Qt::Widget.new(self)

        vbox = Qt::VBoxLayout.new(widget) do |v|
            v.addWidget(@location)
            v.addWidget(split)
        end
 
        @browser = KDE::HTMLPart.new( split )
        @browser.openUrl( KDE::Url.new(@location.text()) )
 
        connect( @browser.browserExtension(),
                 SIGNAL('openUrlRequest(KUrl, KParts::OpenUrlArguments)'),
                 self, SLOT('openUrlRequest(KUrl, KParts::OpenUrlArguments)') )
        self.centralWidget = widget
        setupGUI()
    end
  
    def changeLocation()
        @history.push(@browser.url.url)
        @backAction.enabled = true
        @browser.openUrl(KDE::Url.new(@location.text))
    end
 
    def setUrl( url )
        @location.text = url
        changeLocation()
    end
 
    def openUrlRequest(url, part)
        setUrl(url.url)
    end

    def gotoPreviousPage()
        @location.text = @history.pop() 
        if @history.empty?
            @backAction.enabled = false
        end
        @browser.openUrl( KDE::Url.new(@location.text()) )
    end
 
    def bookLocation()
        iface = Qt::DBusInterface.new("org.kde.BookMarkList", "/", "", Qt::DBusConnection.sessionBus)
        if iface.valid?
            iface.add(@location.text)
        else
            qWarning("Error with DBUS\n")
        end
    end
 
    def fileSetDefaultPage()
        @config.writeEntry("defaultPage", @browser.url.url)
        @config.sync
    end
end

aboutdata = KDE::AboutData.new( "p9", 
                                "Tutorial - p9", KDE.ki18n(""),
                                "1.0", 
                                KDE.ki18n("Step 9 of a simple tutorial"), 
                                KDE::AboutData::License_GPL,
                                KDE.ki18n("(C) 2000, 2001 Antonio Larrosa Jimenez"), 
                                KDE.ki18n(""),
                                "http://devel-home.kde.org/~larrosa/tutorial.html" )
aboutdata.addAuthor(    KDE.ki18n("Antonio Larrosa Jimenez"),
                        KDE.ki18n("Original Developer/Mantainer"), 
                        "larrosa@kde.org",
                        "http://devel-home.kde.org/~larrosa/index.html" )
aboutdata.addAuthor(    KDE.ki18n("Richard Dale"),
                        KDE.ki18n("Ruby port"), 
                        "Richard_Dale@tipitina.demon.co.uk", 
                        "" )    

KDE::CmdLineArgs.init(ARGV, aboutdata)
a = KDE::UniqueApplication.new

if !Qt::DBusConnection::sessionBus.connected?
    $stderr.puts("Cannot connect to the D-BUS session bus.\n" \
                    "To start it, run:\n" \
                    "\teval `dbus-launch --auto-syntax`\n")
    exit(1)
end

if !Qt::DBusConnection.sessionBus.registerService("org.kde.Browser")
    $stderr.puts("%s\n" %  Qt::DBusConnection.sessionBus.lastError.message)
    exit(1)
end

window = Browser.new("Tutorial - p9")
window.resize(300, 200)
window.show

Qt::DBusConnection.sessionBus.registerObject("/", window, Qt::DBusConnection::ExportAllSlots)

a.exec 

