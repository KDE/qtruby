#!/usr/bin/env ruby

require 'Korundum'

# This is an example of a KDE class that has 'k_dcop' slots declarations, but
# isn't a subclass of DCOPObject. The following four methods are added to your
# class:
#
#		  	interfaces()
#			functions()
#			connectDCOPSignal()
#			disconnectDCOPSignal()
#
# See the call to connectDCOPSignal() towards the end of the code as
# an example. The name of the dcop object is always the name of the
# ruby class, and they are Singletons - you can only instantiate one
# of them.
#
# The petshop.rb example in this directory demonstrates more complex
# use of korundum dcop by subclassing a DCOPObject.
#
class MyWidget < KDE::PushButton

	k_dcop	'void mySlot(QString)', 
			'QPoint getPoint(QString)',
			'QMap<QCString,DCOPRef> actionMap()', 
			'QValueList<DCOPRef> windowList()',
			'QValueList<QCString> propertyNames(bool)',
			'KURL::List urlList()',
			'bool isFoo()',
			'bool hasBar()'
				
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
	
	def actionMap()
		map = {}
		map['foobar'] = KDE::DCOPRef.new("myapp", "myobj")
		return map
	end
	
	def windowList()
		list = []
		list[0] = KDE::DCOPRef.new("myapp", "myobj")
		return list
	end
	
	def propertyNames(b)
		return ["thisProperty", "thatProperty"]
	end
	
	def urlList()
		list = []
		list << KDE::URL.new("http://www.kde.org/") << KDE::URL.new("http://dot.kde.org/")
		return list
	end
	
	def isFoo
		true
	end
	
	def hasBar
		true
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
