#!/usr/bin/env ruby

require 'Korundum'

class SenderWidget < KDE::PushButton
	k_dcop_signals 'void testEmitSignal(QString)'
	
	def initialize(parent, name)
		super
		Qt::Object::connect(self, SIGNAL('clicked()'), self, SLOT('doit()'))
	end
	
	slots 'doit()'
	
	def doit()
		emit testEmitSignal("Hello DCOP Slot")
	end
end

about = KDE::AboutData.new("dcopsignal", "DCOPSignalTest", "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::UniqueApplication.new()
signaltest = SenderWidget.new(nil, "foobar") { setText 'DCOP Signal Test' }
a.mainWidget = signaltest
signaltest.show()
a.exec()
