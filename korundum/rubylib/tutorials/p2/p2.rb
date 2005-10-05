require 'Korundum'
 
    about = KDE::AboutData.new("p2", "Hello World", "0.1")
    KDE::CmdLineArgs.init(ARGV, about)
    a = KDE::Application.new()
    hello = Qt::PushButton.new( a.i18n("Hello World !"), nil )
    hello.autoResize = true
        
    a.mainWidget = hello
    hello.show
        
    a.exec