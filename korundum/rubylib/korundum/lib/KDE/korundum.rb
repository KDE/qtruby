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
	include Qt
	
	DCOPMeta = {}

	# An entry for each dcop signal or slot
	# Example 
	#  int foobar(QString,bool)
	#  :name is 'foobar'
	#  :full_name is 'foobar(QString,bool)'
	#  :arg_types is 'QString,bool'
	#  :reply_type is 'int'
	DCOPMember = Struct.new :name, :full_name, :arg_types, :reply_type
	
	# If the class with the 'k_dcop' slots declaration is not a subclass of DCOPObject,
	# then 'dcop_object' holds a instance of DCOPObject for the class to use as a
	# proxy. For subclasses of DCOPObject, 'dcop_object' will always be nil
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
				signal = DCOPClient.normalizeFunctionSignature(signal)
				if signal =~ /^([\w,<>:]*)\s+([^\s]*)\((.*)\)/
					args = DCOPClient.normalizeFunctionSignature($3)
					@k_dcop_signals[$2] = DCOPMember.new($2, $2 + "(" + args + ")", args, $1)
				else
					Qt.qWarning( "Invalid DCOP signal format: '#{signal}'" )
				end
			end
		end
		
		def add_slots(slot_list)
			slot_list.each do |slot|
				if slot =~ /^([\w,<>:]*)\s+([^\s]*)\((.*)\)/
					args = DCOPClient.normalizeFunctionSignature($3)
					@k_dcop[$2] = DCOPMember.new($2, $1 + ' ' + $2 + "(" + args + ")", args, $1)
				else
					Qt.qWarning( "Invalid DCOP slot format: '#{slot}'" )
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

	class KDE::DCOPObject
		def initialize(*k)
			super
		end

		def process(fun, data, replyType, replyData)
			if fun == 'functions()' or fun == 'interfaces()'
				return super
			end
			
			slots = DCOPMeta[@client.class.name].k_dcop
			dcop_slot = slots[fun.sub(/\(.*/, '')]
			if dcop_slot.nil?
				# Can't find an entry for the slot being called? This shouldn't happen..
				return false
			end
			
			replyType << dcop_slot.reply_type
			KDE::dcop_process(	@client, 
								dcop_slot.name, 
								Qt::getMocArguments(fun), 
								data,
								replyType, 
								(replyType == 'void' or replyType == 'ASYNC') ? nil : Qt::getMocArguments(replyType), 
								replyData )
		end

		def interfaces()
			ifaces = super()
			return ifaces << @client.class.name
		end

		def functions()
			funcs = super()
			return funcs + @functions
		end
		
		def functions=(funcs)
			@functions = funcs
		end
		
		# If a ruby class has 'k_dcop' slots declarations, but isn't a 
		# subclass of DCOPObject, then keep an instance of it
		def client=(obj)
			@client = obj
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
				sig = value.reply_type + ' ' + value.name + '(' + value.arg_types + ')'
				funcs << sig 
			end
			meta.changed = false
			if instance.kind_of? DCOPObject
				instance.functions = funcs
				instance.client = instance
				return nil
			else
				if meta.dcop_object.nil?
					# Only ever allocate a single instance of a DCOPObject if the
					# class isn't a subclass of DCOPObject
					meta.dcop_object = DCOPObject.new(instance.class.name)
					meta.dcop_object.client = instance
				end
				meta.dcop_object.functions = funcs
			end
		end

		return meta.dcop_object
	end
	
	class DCOPRef < Qt::Base
		def method_missing(*k)
			# Enables DCOPRef calls to be made like this:
			#
			# dcopRef = DCOPRef.new("dcopslot", "MyWidget")
			# result = dcopRef.getPoint("Hello from dcopcall")
			begin
				# First look for a method in the Smoke runtime.
				# If not found, then throw an exception and try dcop.
				super(*k)
			rescue
				dcopArgs = k[1, k.length-1]
				dcopArgs <<  NoEventLoop << -1
				method = k[0].id2name
				# Make 'parrot.age = 7' a synonym for 'parrot.setAge(7)'
				method = 'set' + method[0,1].upcase + method[1,method.length].sub("=", "") if method =~ /.*[^-+%\/|]=$/
				
				# If the method name contains underscores, convert to camel case
				while method =~ /([^_]*)_(.)(.*)/ 
					method = $1 + $2.upcase + $3
				end
				
				# Get the functions() for this dcop ref and 
				# cache method_name => full_type_signature in a hash
				if @functions.nil?
					@functions = {}
					funcs = call("functions()")
					if funcs.nil?
						return nil
					end
					funcs.each do |func|
						if func =~ /^([\w,<>:]*)\s+(.*)(\(.*\))/
							return_type = $1
							name = $2
							args = $3
							if args =~ / /
								# Remove any arg names
								args.gsub!(/ \w*/, "")
							end
							
							# Make thing? a synonym for isThing() or hasThing()
							if name =~ /^(is|has)(.)(.*)/
								predicate = $2.downcase + $3 + '?'
								if @functions[predicate].nil?
									@functions[predicate] = return_type + " " + name + args
								end
							end
							
							if @functions[name].nil?
								@functions[name] = return_type + " " + name + args
							else
								# If a function name is overloaded, just keep a single name entry in
								# the hash, not all the full type signatures. Then leave dcopTypeNames() 
								# to try and resolve the ambiguous call from the ruby arg types passed.
								@functions.delete(name)
								@functions[name] = name
							end
						end
					end
				end
				
				method = @functions[method]
				if method.nil?
					Qt.qWarning( "DCOPRef: call #{k[0].id2name}() not found" )
					return
				end

				return callExt(method, *dcopArgs)
			end
		end
		
		def dcopTypeNames(*k)
			typeNames = "("
			k.each do |arg|
				if arg.kind_of? Integer
					typeNames << "int,"
				elsif arg.kind_of? Float
					typeNames << "double,"
				elsif arg.kind_of? Array
					typeNames << "QStringList,"
				elsif arg.kind_of? String
					typeNames << "QString,"
				elsif arg.kind_of? Qt::Base
					typeNames << arg.class.name + ","
				elsif arg.instance_of? FalseClass or arg.instance_of? TrueClass
					typeNames << "bool,"
				end
			end
			typeNames.sub!(/,$/, '')
			typeNames.gsub!(/Qt::/, 'Q')
			typeNames.gsub!(/KDE::/, 'K')
			typeNames << ")"
			return typeNames
		end
		
		def call(fun, *k)
			k << NoEventLoop << -1
			callExt(fun, *k)
		end

		def callExt(fun, *k)
			if isNull
				Qt.qWarning( "DCOPRef: call #{fun} on null reference error" )
				return
			end
			sig = fun
			if fun.index('(') == nil
				sig << dcopTypeNames(*k[0, k.length - 2])
			end
			dc = dcopClient()
			if !dc || !dc.isAttached
				Qt.qWarning( "DCOPRef::call():  no DCOP client or client not attached error" )
				return
			end
			if sig =~ /([^\s]*)(\(.*\))/
				full_name = $1+$2
			else
				Qt.qWarning( "DCOPRef: call #{fun} invalid format, expecting '<function_name>(<args>)'" )
				return
			end
			return KDE::dcop_call(	self, 
									full_name, 
									Qt::getMocArguments(full_name),
									*k )
		end

		def send(fun, *k)
			if isNull
				Qt.qWarning( "DCOPRef: send #{fun} on null reference error" )
			end
			sig = fun
			if fun.index('(') == nil
				sig << dcopTypeNames(*k)
			end
			dc = dcopClient()
			if !dc || !dc.isAttached
				Qt.qWarning( "DCOPRef::send():  no DCOP client or client not attached error" )
				return
			end
			if !sig =~ /^([^\s]*)(\(.*\))/
				Qt.qWarning( "DCOPRef: send #{sig} invalid format, expecting '<function_name>(<args>)'" )
				return
			end
			return KDE::dcop_send(	self, 
									fun, 
									Qt::getMocArguments(sig),
									*k )
		end
	end
	
	def CmdLineArgs::init(*k)
		if k.length > 0 and k[0].kind_of?(Array)
			# If init() is passed an array as the first argument, assume it's ARGV.
			# Then convert to a pair of args 'ARGV.length+1, [$0]+ARGV'
			array = k.shift
			super(*([array.length+1] + [[$0] + array] + k))
		else
			super
		end
	end
	
	def MainWindow::RESTORE(klass)
		n = 1
		while MainWindow.canBeRestored(n)
			klass.new.restore(n)
			n += 1
		end
	end
	
	# A sane alternative to the strange looking C++ template version,
	# this takes a variable number of ruby args as classes to restore
	def MainWindow::kRestoreMainWindows(*k)
		n = 1
		while MainWindow.canBeRestored(n)
			className = MainWindow.classNameOfToplevel(n)
			k.each do |klass|
				if klass.name == className
					klass.new.restore(n)
				end
			end
			n += 1
		end
	end
	
	class Application
		def initialize(*k)
			super
			$kapp = self
		end
		
		# Delete the underlying C++ instance after exec returns
		# Otherwise, rb_gc_call_finalizer_at_exit() can delete
		# stuff that KDE::Application still needs for its cleanup.
		def exec
			super
			widgets = topLevelWidgets
			widgets.each {|widget| widget.dispose}
			self.dispose
		end
	end
	
	class UniqueApplication
		def initialize(*k)
			super
			$kapp = self
		end
		
		def exec
			super
			widgets = topLevelWidgets
			widgets.each {|widget| widget.dispose}
			self.dispose
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
