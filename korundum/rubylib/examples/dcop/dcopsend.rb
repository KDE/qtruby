#!/usr/bin/env ruby

require 'Korundum'

class SenderWidget < KDE::PushButton
	def initialize(parent, name)
		super
		Qt::Object::connect(self, SIGNAL('clicked()'), self, SLOT('doit()'))
	end
	
	slots 'doit()'
	
	def doit()
		dcopRef = KDE::DCOPRef.new("dcopslot", "MyWidget")
		dcopRef.send("mySlot(QString)", "Hello from dcopsend")
	end
end

about = KDE::AboutData.new("dcopsend", "DCOPSendTest", "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::Application.new()
sender = SenderWidget.new(nil, "senderwidget") { setText 'DCOP Send Test' }
a.setMainWidget(sender)
sender.show()
a.exec()
