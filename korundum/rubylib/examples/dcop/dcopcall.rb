#!/usr/bin/env ruby

require 'Korundum'

class SenderWidget < KDE::PushButton
	def initialize(parent, name)
		super
		Qt::Object::connect(self, SIGNAL('clicked()'), self, SLOT('doit()'))
	end
	
	slots 'doit()'
	
	def doit()
		puts "In doit.."
		dcopRef = KDE::DCOPRef.new("dcopslot", "MyWidget")
		result = dcopRef.call("QPoint getPoint(QString)", "Hello from dcopsend")
		puts "result class: #{result.class.name} x: #{result.x} y: #{result.y}"
	end
end

about = KDE::AboutData.new("dcopcall", "DCOPCallTest", "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::UniqueApplication.new()
calltest = SenderWidget.new(nil, "calltest") { setText 'DCOP Call Test' }
a.setMainWidget(calltest)
calltest.show()
a.exec()
