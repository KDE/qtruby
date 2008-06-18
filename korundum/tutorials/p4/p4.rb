require 'korundum4'
require 'khtml'

class MainWindow < KDE::MainWindow
    slots   :changeLocation, 
            'openUrlRequest(KUrl, KParts::OpenUrlArguments)'
 
    def initialize(name)
        super(nil)
        self.objectName = name
        self.caption = "KDE Tutorial - p4"

        filemenu = Qt::Menu.new(i18n("&File"))
        filemenu.addAction(i18n("&Quit"), $kapp, SLOT(:quit))
        about = i18n("p4 1.0\n\n" +
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
 
        @browser = KDE::HTMLPart.new
        @browser.openUrl(KDE::Url.new(@location.text))

        widget = Qt::Widget.new(self)

        vbox = Qt::VBoxLayout.new(widget) do |v|
            v.addWidget(@location)
            v.addWidget(@browser.widget)
        end

        connect( @location , SIGNAL(:returnPressed),
                    self, SLOT(:changeLocation) )
 
        connect( @browser.browserExtension,
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
end

about = KDE::AboutData.new("p4", "Tutorial - p4", KDE.ki18n(""), "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::Application.new

window = MainWindow.new("Tutorial - p4")
window.resize(300, 200)
window.show

a.exec