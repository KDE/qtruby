require 'korundum4'
 
class BookMarkList < Qt::TableWidget
    slots 'add(QString)',
          'setUrlInBrowser(QTableWidgetItem*)'
 
    def initialize()
        super(0, 1)
        self.objectName = "Bookmarks"
        self.horizontalHeaderLabels = [i18n("My Bookmarks")]
        horizontalHeader().resizeSection(0, 250)
        connect( self, SIGNAL('itemClicked(QTableWidgetItem*)'), 
                 self, SLOT('setUrlInBrowser(QTableWidgetItem*)'))
    end
         
    def add(s)
        setRowCount(rowCount + 1)
        setItem(rowCount - 1, 0, Qt::TableWidgetItem.new(s))
    end

    def setUrlInBrowser(item)
        iface = Qt::DBusInterface.new("org.kde.Browser", "/", "", Qt::DBusConnection.sessionBus)
        if iface.valid?
            iface.setUrl(item.text)
        else
            qWarning("Error with DBUS\n")
        end
    end
end
    
about = KDE::AboutData.new("p8", "Tutorial - p8", KDE.ki18n(""), "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::UniqueApplication.new

if !Qt::DBusConnection::sessionBus.connected?
    $stderr.puts("Cannot connect to the D-BUS session bus.\n" \
                    "To start it, run:\n" \
                    "\teval `dbus-launch --auto-syntax`\n")
    exit(1)
end
    
if !Qt::DBusConnection.sessionBus.registerService("org.kde.BookMarkList")
    $stderr.puts("%s\n" %  Qt::DBusConnection.sessionBus.lastError.message)
    exit(1)
end

mylist = BookMarkList.new
mylist.resize(300, 200)
a.topWidget = mylist
mylist.show

Qt::DBusConnection.sessionBus.registerObject("/", mylist, Qt::DBusConnection::ExportAllSlots)

a.exec 