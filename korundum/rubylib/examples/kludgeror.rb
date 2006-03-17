#!/usr/bin/env ruby
# A quick and dirty web browser demonstrating the direct instantiation 
# of a KHTML part. Needless to say: this is NOT a programming paragon :)
# -- gg.
# AK - ported to ruby

require 'Korundum'

opt =     [ [ "+[url]",      "An URL to open at startup.",         ""            ],
            [ "z",           "A dummy binary option.",             ""            ],
            [ "baz <file>",  "A long option with arg.",            ""            ],
            [ "boz <file>",  "Same as above with default value",   "default.txt" ],
          ]            

Qt::Internal::setDebug Qt::QtDebugChannel::QTDB_ALL
Qt.debug_level = Qt::DebugLevel::High

about = KDE::AboutData.new("kludgeror", "Kludgeror", "0.1", "A basic web browser")
KDE::CmdLineArgs::init(ARGV.length + 1, [$0] + ARGV, about)
KDE::CmdLineArgs::addCmdLineOptions opt
args = KDE::CmdLineArgs::parsedArgs

a = KDE::Application.new # BUG, application shouldn't be needed at the top, lets fix this...

class PartHolder < Qt::Object
    signals "setLocBarText(const QString&)"
    slots "reload()", "goToURL(const QString&)", "back()", "openURL(const KURL&)" # BUG - the slots should be normalize wrt spaces by the lib

    attr_accessor :part, :history

    def initialize part, *k
        super(*k)
        @part = part 
        @history = []
    end

    def openURL url
        puts "in openURL #{url}"
        @part.openURL url
        # BUG - a non existant slot emit says horrible things, nothing interesting for user.. very confusing
        # BUG - signal emitting is *very* wrong
        # emit setLocBarText(url.url)
        @history.unshift url unless url == @history[0]
    end

    def reload
        @part.openURL @part.url
    end

    def goToURL(url)
        url = "http://#{url}" unless url =~ /^\w*:/
        openURL KDE::URL.new(url)
    end

    def back
        return unless @history.length > 1
        @history.shift
        openURL @history[0] unless @history.empty?
    end
end

LOC_ED  = 322
ERASE_B = 323
BACK_B  = 324

url = (args.count > 1) ? args.url(0) : KDE::URL.new("http://loki:8080/xml/index.xml")

puts "Dummy z option activated." if args.isSet "z"
puts "Dummy baz option has value: #{args.getOption "baz"}" if args.isSet "baz"
# puts "Dummy boz option has value: #{args.getOption "boz"}" if args.isSet "boz" # B0rked?

toplevel = KDE::MainWindow.new
doc = KDE::HTMLPart.new toplevel, nil, toplevel, nil, 1
# doc = KDE::HTMLPart.new toplevel, nil, toplevel, nil, 1 # KDE::HTMLPart::BrowserViewGUI
ph = PartHolder.new doc

Qt::Object::connect doc.browserExtension, SIGNAL("openURLRequest(const KURL&, const KParts::URLArgs&)"),
                    ph, SLOT("openURL(const KURL&)") # BUG this slot must be screwing up wrt marshalling?

ph.openURL url
toplevel.setCentralWidget doc.widget
toplevel.resize 700, 500

begin
  d, viewMenu, fileMenu, locBar, e = nil
  d = doc.domDocument
    viewMenu = d.documentElement.firstChild.childNodes.item(2).toElement
        e = d.createElement "action"
        e.setAttribute "name", "debugRenderTree"
    viewMenu.appendChild e
        e = d.createElement "action"
        e.setAttribute "name", "debugDOMTree"
    viewMenu.appendChild e
    fileMenu = d.documentElement.firstChild.firstChild.toElement
    fileMenu.appendChild d.createElement("separator")
        e = d.createElement "action"
        e.setAttribute "name", "exit"
    fileMenu.appendChild e
    locBar = d.createElement "toolbar"
    locBar.setAttribute "name", "locationBar"
        e = d.createElement "action"
        e.setAttribute "name", "reload"
    locBar.appendChild e
  d.documentElement.appendChild locBar
end

a1 = KDE::Action.new( "Reload", "reload", KDE::Shortcut.new(Qt::Key_F5), ph, SLOT("reload()"),  doc.actionCollection, "reload" )
a2 = KDE::Action.new( "Exit",   "exit",   KDE::Shortcut.new(0),          a,  SLOT("quit()"),    doc.actionCollection, "exit" )

toplevel.guiFactory.addClient doc

locBar = toplevel.toolBar("locationBar");
locBar.insertButton "back", BACK_B, SIGNAL("clicked()"), 
                    ph, SLOT("back()"), 1, "Go back"
locBar.insertLined url.url, LOC_ED, SIGNAL("returnPressed(const QString&)"), ph, SLOT("goToURL(const QString&)"), 1, "Location"
locBar.insertButton "locationbar_erase", ERASE_B, SIGNAL("clicked()"), 
                    locBar.getLined(LOC_ED), SLOT("clear()"), 1, "Erase the location bar's content", 2
locBar.setItemAutoSized LOC_ED, 1
locBar.getLined(LOC_ED).createPopupMenu
comp = locBar.getLined(LOC_ED).completionObject
comp.setCompletionMode KDE::GlobalSettings::CompletionPopupAuto

Qt::Object::connect(locBar.getLined(LOC_ED), SIGNAL("returnPressed(const QString&)"),
                    comp, SLOT("addItem(const QString&)"))
Qt::Object::connect(ph, SIGNAL("setLocBarText(const QString&)"), # BUG - once again...
                    locBar.getLined(LOC_ED), SLOT("setText(const QString&)"))

doc.setJScriptEnabled true
doc.setJavaEnabled true
doc.setPluginsEnabled true
doc.setURLCursor Qt::Cursor.new(Qt::PointingHandCursor)

a.setTopWidget doc.widget

Qt::Object::connect doc, SIGNAL("setWindowCaption(const QString&)"),
                    doc.widget.topLevelWidget, SLOT("setCaption(const QString&)")
toplevel.show

a.exec
