require 'Korundum'
  
class BookMarkList < KDE::ListView
    k_dcop 'void add(QString)'
	
    slots 'setURLInBrowser(QListViewItem *)'
 
    def initialize()
       super(nil, "Bookmarks")
       addColumn( i18n("My Bookmarks") );
       connect( self, SIGNAL('clicked(QListViewItem *)'), 
		    self, SLOT('setURLInBrowser(QListViewItem *)'))
    end
 
    def add( s )
        insertItem( KDE::ListViewItem.new( self , s ) )
    end
 
    def setURLInBrowser( item )
        if item.nil? then return end
        dcopRef = KDE::DCOPRef.new("p7", "Browser")
        if ! dcopRef.setURL(item.text(0))
            qWarning("Error with DCOP\n")
        end
    end
end

    about = KDE::AboutData.new("p8", "Tutorial - p8", "0.1")
    KDE::CmdLineArgs.init(ARGV, about)
    a = KDE::UniqueApplication.new()
	
    mylist = BookMarkList.new
    mylist.resize( 300, 200 )
	
    a.mainWidget = mylist
    mylist.show
	
    a.exec 

