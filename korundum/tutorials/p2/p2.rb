require 'korundum4'
 
about = KDE::AboutData.new("p2", "Hello World", KDE.ki18n(""), "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::Application.new
hello = Qt::PushButton.new(KDE.i18n("Hello World !"), nil)

a.topWidget = hello
hello.show

a.exec