#!/usr/bin/env ruby

require 'Korundum'

class MyWidget < KDE::PushButton
	k_dcop 'mySlot()'
	
	def initialize(parent, name)
		super(parent, name)
	end
	
	def mySlot()
		puts "In mySlot"
	end
end

about = KDE::AboutData.new("dcopslot", "two", "three")
KDE::CmdLineArgs.init(1, ["dcopslot"], about)
a = KDE::Application.new()
hello = MyWidget.new(nil, "foobar") { setText "Hello World" }
a.mainWidget = hello

hello.show()
a.exec()
