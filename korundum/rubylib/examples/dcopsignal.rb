#!/usr/bin/env ruby

require 'Korundum'

class MyWidget < KDE::PushButton
	k_dcop_signals 'mySignal()'
	
	def initialize(parent, name)
		super(parent, name)
	end
	
	def emitSignal()
		emit mySignal()
	end
end

about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["dcopsignal"], about)
a = KDE::Application.new()
hello = MyWidget.new(nil, "foobar") { setText "Hello World" }
#hello = KDE::PushButton.new(nil, "foobar") { setText "Hello World" }
a.setMainWidget(hello)
hello.show()
hello.emitSignal()
a.exec()
