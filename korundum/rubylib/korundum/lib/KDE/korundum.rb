=begin
/***************************************************************************
                          Korundum.rb  -  KDE specific ruby runtime, dcop etc.
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003-2004 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
=end

module KDE
	DCOPMeta = {}

	# An entry for each dcop signal or slot
	# Example 
	#  int foobar(QString,bool)
	#  :name is 'foobar'
	#  :full_name is 'int foobar(QString,bool)'
	#  :arg_types is 'QString,bool'
	#  :reply_type is 'int'
	DCOPMember = Struct.new :name, :full_name, :arg_types, :reply_type
	
	class DCOPMetaInfo
		attr_accessor :dcop_object, :changed
		attr_reader :k_dcop_signals, :k_dcop
		
		def initialize(aClass)
			DCOPMeta[aClass.name] = self
			@dcop_object = nil
			@k_dcop_signals = {}
			@k_dcop = {}
			@changed = false
		end
		
		def add_signals(signal_list)
			signal_list.each do |signal|
				if signal =~ /^(.*)\s([^\s]*)\((.*)\)/
					@k_dcop_signals[$2] = DCOPMember.new($2, signal, $3, $1)
				end
			end
		end
		
		def add_slots(slot_list)
			slot_list.each do |slot|
				if slot =~ /^(.*)\s([^\s]*)\((.*)\)/
					@k_dcop[$2] = DCOPMember.new($2, slot, $3, $1)
				end
			end
		end
	end

	def hasDCOPSignals(aClass)
		classname = aClass.name if aClass.is_a? Module
		meta = DCOPMeta[classname]
		return !meta.nil? && meta.k_dcop_signals.length > 0
	end

	def hasDCOPSlots(aClass)
		classname = aClass.name if aClass.is_a? Module
		meta = DCOPMeta[classname]
		return !meta.nil? && meta.k_dcop.length > 0
	end

	def getDCOPSignalNames(aClass)
		classname = aClass.name if aClass.is_a? Module
		signals = DCOPMeta[classname].k_dcop_signals
		return signals.keys
	end

	def fullSignalName(instance, signalName)
		classname = instance.class.name if instance.class.is_a? Module
		signals = DCOPMeta[classname].k_dcop_signals
		return signals[signalName].full_name
	end

	class RubyDCOPObject < KDE::DCOPObject
		def initialize(instance, functions)
			super(instance.class.name)
			@instance = instance
			@functions = functions
		end

		def process(fun, data, replyType, replyData)
			if fun == 'functions()' or fun == 'interfaces()'
				return super
			end
			
			slots = DCOPMeta[@instance.class.name].k_dcop
			dcop_slot = slots[fun.sub(/\(.*/, '')]
			if dcop_slot.nil?
				# Can't find an entry for the slot being called? This shouldn't happen..
				return false
			end
			
			replyType << dcop_slot.reply_type
			KDE::dcop_process(	@instance, 
								dcop_slot.name, 
								Qt::getMocArguments(fun), 
								data, 
								(replyType == 'void' or replyType == 'ASYNC') ? nil : Qt::getMocArguments(replyType), 
								replyData )
		end

		def interfaces()
			ifaces = super()
			return ifaces << @instance.class.name
		end

		def functions()
			functions = super()
			return functions + @functions
		end
	end

	# If a class contains a k_dcop slots list declaration, then create a DCOPObject
	# associated with it	
	def createDCOPObject(instance)
		meta = DCOPMeta[instance.class.name]
		return nil if meta.nil?

		if meta.dcop_object.nil? or meta.changed
			funcs = []
			meta.k_dcop.each_value do |value| 
				func_name = value.name + '(' + value.arg_types + ')'
				funcs << func_name 
			end
			meta.dcop_object = RubyDCOPObject.new(instance, funcs)
			meta.changed = false
		end

		meta.dcop_object
	end
	
	class DCOPRef < Qt::Base
		def call(fun, *k)
			k << NoEventLoop << -1
			callExt(fun, *k)
		end

		def callExt(fun, *k)
			if isNull
				puts( "DCOPRef: call #{fun} on null reference error" )
			end
			dc = dcopClient()
			if !dc || !dc.isAttached
				puts( "DCOPRef::call():  no DCOP client or client not attached error" )
			end
			puts "fun: #{fun}"
			if fun =~ /^(.*)\s([^\s]*)(\(.*\))/
				reply_type = $1
				full_name = $2+$3
			else
				puts( "DCOPRef: call #{fun} invalid format, expecting '<reply_type> <function_name>(<args>)'" )
			end
			return KDE::dcop_call(	self, 
									full_name, 
									Qt::getMocArguments(full_name),
									reply_type, 
									(reply_type == 'void' or reply_type == 'ASYNC') ? nil : Qt::getMocArguments(reply_type), 
									*k )
		end

		def send(fun, *k)
			if isNull
				puts( "DCOPRef: send #{fun} on null reference error" )
			end
			dc = dcopClient()
			if !dc || !dc.isAttached
				puts( "DCOPRef::send():  no DCOP client or client not attached error" )
			end
			if !fun =~ /^([^\s]*)(\(.*\))/
				puts( "DCOPRef: send #{fun} invalid format, expecting '<function_name>(<args>)'" )
			end
			return KDE::dcop_send(	self, 
									fun, 
									Qt::getMocArguments(fun),
									*k )
		end
	end
	
	def CmdLineArgs::init(*k)
		if k.length > 0 and k[0].kind_of?(Array)
			# If init() is passed an array as the first argument, assume it's ARGV.
			# Then convert to a pair of args 'ARGV.length+1, [$0]+ARGV'
			array = k.shift
			super *([array.length+1] + [[$0] + array] + k)
		else
			super
		end
	end
end

class Module
	include KDE

	def k_dcop_signals(*signal_list)
		meta = DCOPMeta[self.name] || DCOPMetaInfo.new(self)
		meta.add_signals(signal_list)
		meta.changed = true
	end

	def k_dcop(*slot_list)
		meta = DCOPMeta[self.name] || DCOPMetaInfo.new(self)
		meta.add_slots(slot_list)
		meta.changed = true
	end
end
