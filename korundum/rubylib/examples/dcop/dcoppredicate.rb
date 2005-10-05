#!/usr/bin/env ruby

require 'Korundum'
include KDE

class SenderWidget < PushButton
	def initialize(parent, name)
		super
		connect(self, SIGNAL('clicked()'), self, SLOT('doit()'))
	end
	
	slots 'doit()'
	
	def doit()
		dcopRef = DCOPRef.new("dcopslot", "MyWidget")
		#
		# A synonym for isFoo()
		result = dcopRef.foo?
		if result.nil?
			puts "DCOP predicate failed"
		else
			puts "foo? is #{result}"
		end
		
		# A synonym for hasBar()
		result = dcopRef.bar?
		if result.nil?
			puts "DCOP predicate failed"
		else
			puts "bar? is #{result}"
		end
	end
end

about = AboutData.new("dcoppredicate", "DCOP Predicate Test", "0.1")
CmdLineArgs.init(ARGV, about)
a = UniqueApplication.new
calltest = SenderWidget.new(nil, "predicatetest") { setText 'DCOP Predicate Test' }
a.mainWidget = calltest
calltest.show
a.exec
