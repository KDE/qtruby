require 'korundum4'
require 'khtml'

class MainWindow < KDE::MainWindow
    slots   :changeLocation, 
            'openUrlRequest(KUrl, KParts::OpenUrlArguments)',
            :bookLocation
 
    def initialize(name)
        super(nil)
        self.objectName = name
        self.caption = "KDE Tutorial - p5"

        filemenu = Qt::Menu.new(i18n("&File"))
        filemenu.addAction(i18n("&Quit"), $kapp, SLOT(:quit))
        about =i18n( "p5 1.0\n\n" +
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
 
        @location = Qt::LineEdit.new
        @location.text = "http://localhost"
 
        connect( @location , SIGNAL(:returnPressed),
                    self, SLOT(:changeLocation) )
 
        split = Qt::Splitter.new
        split.opaqueResize = true

        widget = Qt::Widget.new(self)

        vbox = Qt::VBoxLayout.new(widget) do |v|
            v.addWidget(@location)
            v.addWidget(split)
        end
 
        bookmark = Qt::PushButton.new(i18n("Add to Bookmarks"), split)
 
        connect(bookmark, SIGNAL(:clicked), self, SLOT(:bookLocation) )
 
        @browser = KDE::HTMLPart.new(split)
        @browser.openUrl(KDE::Url.new(@location.text))
 
        connect( @browser.browserExtension(),
                 SIGNAL('openUrlRequest(KUrl, KParts::OpenUrlArguments)'),
                 self, SLOT('openUrlRequest(KUrl, KParts::OpenUrlArguments)') )
        self.centralWidget = widget
    end
 
 
    def changeLocation()
        @browser.openUrl(KDE::Url.new(@location.text))
    end

    def openUrlRequest(url, part)
        @location.text = url.url
        changeLocation()
    end
 
    def bookLocation()
		iface = Qt::DBusInterface.new("org.kde.BookMarkList", "/", "", Qt::DBusConnection.sessionBus)
		if iface.valid?
			iface.add(@location.text)
		else
            qWarning("Error with DBUS\n")
		end
    end
end

about = KDE::AboutData.new("p5", "Tutorial - p5", KDE.ki18n(""), "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::Application.new()

if !Qt::DBusConnection.sessionBus.connected?
	$stderr.puts("Cannot connect to the D-BUS session bus.\n" \
	                "To start it, run:\n" \
	                "\teval `dbus-launch --auto-syntax`\n")
	exit(1)
end

window = MainWindow.new("Tutorial - p5")
window.resize(300, 200)
window.show

a.exec 
