require 'Korundum'
 
class BookMarkList < KDE::ListView
    k_dcop 'void add( QString )'
 
    def initialize()
       super(nil, "Bookmarks")
       addColumn( i18n("My Bookmarks") );
    end
         
    def add( s )
        insertItem( KDE::ListViewItem.new( self , s ) )
    end
end
    
    about = KDE::AboutData.new("p6", "Tutorial - p6", "0.1")
    KDE::CmdLineArgs.init(ARGV, about)
    a = KDE::UniqueApplication.new()
        
    mylist = BookMarkList.new
    mylist.resize( 300, 200 )
        
    a.mainWidget = mylist
    mylist.show
        
    a.exec 