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
		# Note that there are three different ways to make a DCOP call():
		# 	1) result = dcopRef.call("getPoint(QString)", "Hello from dcopcall")
		# 	2) result = dcopRef.call("getPoint", "Hello from dcopcall")
		#	3) result = dcopRef.getPoint("Hello from dcopcall")
		#
		result = dcopRef.getPoint("Hello from dcopcall")
		if result.nil?
			puts "DCOP call failed"
		else
			puts "result class: #{result.class.name} x: #{result.x} y: #{result.y}"
		end
	end
end

about = AboutData.new("dcopcall", "DCOP Call Test", "0.1")
CmdLineArgs.init(ARGV, about)
a = UniqueApplication.new
calltest = SenderWidget.new(nil, "calltest") { setText 'DCOP Call Test' }
a.mainWidget = calltest
calltest.show
a.exec
