#!/usr/bin/env ruby

require 'Korundum'

class MyWidget < KDE::PushButton
	k_dcop 'void mySlot(QString)', 'QPoint getPoint(QString)'
	
	def initialize(parent, name)
		super
	end
	
	def mySlot(greeting)
		puts "greeting: #{greeting}"
	end
	
	def getPoint(msg)
		puts "message: #{msg}"
		return Qt::Point.new(50, 100)
	end
end

about = KDE::AboutData.new("dcopslot", "dcopSlotTest", "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::UniqueApplication.new()
slottest = MyWidget.new(nil, "mywidget") { setText "DCOP Slot Test" }
a.mainWidget = slottest
slottest.caption = a.makeStdCaption("DCOP Slot Test")
result = slottest.connectDCOPSignal("dcopsignal", "SenderWidget", "testEmitSignal(QString)", "mySlot(QString)", true)
puts "result: #{result}"

slottest.show()
# Qt::Internal::setDebug Qt::QtDebugChannel::QTDB_ALL
a.exec()
