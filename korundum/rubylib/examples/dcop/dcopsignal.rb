#!/usr/bin/env ruby

require 'Korundum'

class SenderWidget < KDE::PushButton
	k_dcop_signals 'void testEmitSignal(QString)'
	
	def initialize(parent, name)
		super
		Qt::Object::connect(self, SIGNAL('clicked()'), self, SLOT('do_it()'))
	end
	
	slots 'do_it()'
	
	def do_it()
		puts "In do_it.."
		emit testEmitSignal("Hello DCOP Slot")
	end
end

about = KDE::AboutData.new("dcopsignal", "dcopSignalTest", "0.1")
KDE::CmdLineArgs.init(ARGV, about)
a = KDE::Application.new()
hello = SenderWidget.new(nil, "foobar") { setText 'DCOP Signal Test' }
a.setMainWidget(hello)
hello.show()
a.exec()
