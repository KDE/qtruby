#!/usr/bin/env ruby

require 'Korundum'

class MyWidget < KDE::PushButton
	k_dcop 'QPoint mySlot(QString)'
	
	def initialize(parent, name)
		super
	end
	
	def mySlot(greeting)
		puts "greeting: #{greeting}"
		return Qt::Point.new(50, 100)
	end
end

about = KDE::AboutData.new("dcopslot", "dcopSlotTest", "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::Application.new()
hello = MyWidget.new(nil, "foobar") { setText "DCOP Slot Test" }
a.mainWidget = hello
hello.caption = a.makeStdCaption("DCOP Slot Test")
result = hello.connectDCOPSignal("", "SenderWidget", "testEmitSignal(QString)", "mySlot(QString)", false)
puts "result: #{result}"

hello.show()
# Qt::Internal::setDebug Qt::QtDebugChannel::QTDB_ALL
a.exec()
