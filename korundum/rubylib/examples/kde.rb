#!/usr/bin/env ruby -w

require 'Korundum'

about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["four"], about)
a = KDE::Application.new()
hello = KDE::PushButton.new(nil) { setText "Hello World" }
a.setMainWidget(hello)
dcop = KDE::DCOPObject.new()
hello.show()
a.exec()
