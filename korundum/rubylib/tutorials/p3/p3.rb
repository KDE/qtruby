require 'Korundum'
 
class MainWindow < KDE::MainWindow
    slots 'fileOpen()', 'fileSave()'
 
    def initialize( name )
        super(nil, name)
        setCaption("KDE Tutorial - p3")

        filemenu = Qt::PopupMenu.new
        filemenu.insertItem( i18n( "&amp;Open" ), self, SLOT('fileOpen()') )
        filemenu.insertItem( i18n( "&amp;Save" ), self, SLOT('fileSave()') )
        filemenu.insertItem( i18n( "&amp;Quit" ), $kapp, SLOT('quit()') )
		
        about =
            i18n("p3 1.0\n\n" +
                 "(C) 1999-2002 Antonio Larrosa Jimenez\n" +
                 "larrosa@kde.org\t\tantlarr@supercable.es\n" +
                 "Malaga (Spain)\n\n" +
                 "Simple KDE Tutorial\n" +
                 "This tutorial comes with ABSOLUTELY NO WARRANTY\n" +
                 "This is free software, and you are welcome to redistribute it\n" +
                 "under certain conditions\n")
        helpmenu = helpMenu( about )
		
        menu = menuBar()
        menu.insertItem( i18n( "&amp;File" ), filemenu )
        menu.insertSeparator()
        menu.insertItem( i18n( "&amp;Help" ), helpmenu )
 
        hello = Qt::TextView.new(
        i18n("<H2>Hello World !</H2><BR>This is a simple" +
            " window with <I><font size=5><B>R<font color=red" +
            " size=5>ich </font><font color=blue size=5>Text" +
            "</font></B></I> capabilities<BR>Try to resize" +
            " this window, all this is automatic !"), "", self )
        setCentralWidget(hello)
    end
 
    def fileOpen()
        filename = KDE::FileDialog.getOpenURL( nil, "*", self )
        msg = i18n("Now this app should open the url #{filename.url()}") 
        KDE::MessageBox.information( nil, msg, i18n( "Information" ), 
		"fileOpenInformationDialog" )
    end
 
    def fileSave()
        filename = KDE::FileDialog.getSaveURL( nil, "*", self )
    end
end

    about = KDE::AboutData.new("p3", "Tutorial - p3", "0.1")
    KDE::CmdLineArgs.init(ARGV, about)
    a = KDE::Application.new()
    window = MainWindow.new( "Tutorial - p3" )
    window.resize( 400, 300 )
	
    a.mainWidget = window
    window.show
	
    a.exec 
