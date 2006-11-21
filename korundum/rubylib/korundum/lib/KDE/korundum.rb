=begin
/***************************************************************************
                          Korundum.rb  -  KDE specific ruby runtime, dcop etc.
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003-2006 by Richard Dale
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
					qWarning( "Invalid DCOP signal format: '#{signal}'" )
				end
			end
		end
		
		def add_slots(slot_list)
			slot_list.each do |slot|
				if slot =~ /^([\w,<>:]*)\s+([^\s]*)\((.*)\)/
					args = DCOPClient.normalizeFunctionSignature($3)
					@k_dcop[$2] = DCOPMember.new($2, $1 + ' ' + $2 + "(" + args + ")", args, $1)
				else
					qWarning( "Invalid DCOP slot format: '#{slot}'" )
				end
			end
		end
	end # DCOPMetaInfo

	def KDE.hasDCOPSignals(aClass)
		classname = aClass.name if aClass.is_a? Module
		meta = DCOPMeta[classname]
		return !meta.nil? && meta.k_dcop_signals.length > 0
	end

	def KDE.hasDCOPSlots(aClass)
		classname = aClass.name if aClass.is_a? Module
		meta = DCOPMeta[classname]
		return !meta.nil? && meta.k_dcop.length > 0
	end

	def KDE.getDCOPSignalNames(aClass)
		classname = aClass.name if aClass.is_a? Module
		signals = DCOPMeta[classname].k_dcop_signals
		return signals.keys
	end

	module Internal
		def Internal.fullSignalName(instance, signalName)
			classname = instance.class.name if instance.class.is_a? Module
			signals = DCOPMeta[classname].k_dcop_signals
			return signals[signalName].full_name
		end
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
								Qt::Internal::getMocArguments(fun), 
								data,
								replyType, 
								(replyType == 'void' or replyType == 'ASYNC') ? nil : Qt::Internal::getMocArguments(replyType), 
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
		
		def inspect
			str = super
			if @functions != nil
				str.sub(/>$/, " objId=%s, functions=Array (%d element(s))>" % [objId.inspect, functions.length])
			end
		end
		
		def pretty_print(pp)
			str = to_s
			if @functions != nil
				pp.text str.sub(/>$/, "\n objId=%s,\n functions=Array (%d element(s))>" % [objId.inspect, functions.length])
			end
		end
	end

	# If a class contains a k_dcop slots list declaration, then create a DCOPObject
	# associated with it	
	def KDE.createDCOPObject(instance)
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
					qWarning( "DCOPRef: call #{k[0].id2name}() not found" )
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
				qWarning( "DCOPRef: call #{fun} on null reference error" )
				return
			end
			sig = fun
			if fun.index('(') == nil
				sig << dcopTypeNames(*k[0, k.length - 2])
			end
			dc = dcopClient()
			if !dc || !dc.isAttached
				qWarning( "DCOPRef::call():  no DCOP client or client not attached error" )
				return
			end
			if sig =~ /([^\s]*)(\(.*\))/
				full_name = $1+$2
			else
				qWarning( "DCOPRef: call #{fun} invalid format, expecting '<function_name>(<args>)'" )
				return
			end
			return KDE::dcop_call(	self, 
									full_name, 
									Qt::Internal::getMocArguments(full_name),
									*k )
		end

		def send(fun, *k)
			if isNull
				qWarning( "DCOPRef: send #{fun} on null reference error" )
			end
			sig = fun
			if fun.index('(') == nil
				sig << dcopTypeNames(*k)
			end
			dc = dcopClient()
			if !dc || !dc.isAttached
				qWarning( "DCOPRef::send():  no DCOP client or client not attached error" )
				return
			end
			if !sig =~ /^([^\s]*)(\(.*\))/
				qWarning( "DCOPRef: send #{sig} invalid format, expecting '<function_name>(<args>)'" )
				return
			end
			return KDE::dcop_send(	self, 
									fun, 
									Qt::Internal::getMocArguments(sig),
									*k )
		end

		def methods
			if @functions.nil?
				functions()
			end

			result = super + @functions.keys.map {|k| k.sub(/^(set)([A-Z])(.*)/) { $2.downcase + $3 + '=' } }
			return result.uniq
		end

		def inspect
			str = super
			str.sub(/>$/, " app=%s, obj=%s>" % [app.inspect, obj.inspect])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n app=%s,\n obj=%s>" % [app.inspect, obj.inspect])
		end

		def type(*args)
			method_missing(:type, *args)
		end
	end
	
	def CmdLineArgs::init(*k)
		if k.length > 0
			if k[0].kind_of? Array
				# If init() is passed an array as the first argument, assume it's ARGV.
				# Then convert to a pair of args 'ARGV.length+1, [$0]+ARGV'
				array = k.shift
				super(*([array.length+1] + [[$0] + array] + k))
			elsif k[0].kind_of? KDE::AboutData
				super(1, [$0], k[0])
			end
		else
			super
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

	class AboutData
		def inspect
			str = super
			str.sub!(/>$/, " appName=%s, copyrightStatement=%s, programName=%s, version=%s, shortDescription=%s, homepage=%s, bugAddress=%s>" %
							[appName.inspect, copyrightStatement.inspect, programName.inspect, version.inspect,
							shortDescription.inspect, homepage.inspect, bugAddress.inspect] )
			length = authors.length
			if length > 0
				str.sub!(/>$/, ", authors=Array (%d element(s))>" % length)
			end
			length = credits.length
			if length > 0
				str.sub!(/>$/, ", credits=Array (%d element(s))>" % length)
			end
			length = translators.length
			if length > 0
				str.sub!(/>$/, ", translators=Array (%d element(s))>" % length)
			end
			return str
		end
		
		def pretty_print(pp)
			str = to_s
			str.sub!(/>$/, "\n appName=%s,\n copyrightStatement=%s,\n programName=%s,\n version=%s,\n shortDescription=%s,\n homepage=%s,\n bugAddress=%s>" % 
							[appName.inspect, copyrightStatement.inspect, programName.inspect, version.inspect,
							shortDescription.inspect, homepage.inspect, bugAddress.inspect] )
			length = authors.length
			if length > 0
				str.sub!(/>$/, ",\n authors=Array (%d element(s))>" % length)
			end
			length = credits.length
			if length > 0
				str.sub!(/>$/, ",\n credits=Array (%d element(s))>" % length)
			end
			length = translators.length
			if length > 0
				str.sub!(/>$/, ",\n translators=Array (%d element(s))>" % length)
			end
			pp.text str
		end
	end
	
	class AboutPerson
		def inspect
			str = super
			str.sub(/>$/, " emailAddress=%s, name=%s, task=%s, webAddress=%s>" % 
						[emailAddress.inspect, name.inspect, task.inspect, webAddress.inspect] )
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n emailAddress=%s,\n name=%s,\n task=%s,\n webAddress=%s>" % 
						[emailAddress.inspect, name.inspect, task.inspect, webAddress.inspect] )
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end
	
	class AboutTranslator
		def inspect
			str = super
			str.sub(/>$/, " emailAddress=%s, name=%s>" % 
						[emailAddress.inspect, name.inspect] )
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n emailAddress=%s,\n name=%s>" % 
						[emailAddress.inspect, name.inspect] )
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end

	class AccelShortcutList
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ActionPtrShortcutList
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ActionShortcutList
		def name(*args)
			method_missing(:name, *args)
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
			method_missing(:exec)
			self.dispose
			Qt::Internal.application_terminated = true
		end
	end

	class Archive
		def open(*args)
			method_missing(:open, *args)
		end
	end

	class ArchiveEntry
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class BookmarkDrag
		def format(*args)
			method_missing(:format, *args)
		end
	end

	class CModule
		def load(*args)
			method_missing(:load, *args)
		end
	end

	class Catalogue
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ColorDrag
		def format(*args)
			method_missing(:format, *args)
		end
	end

	class CustomMenuEditor
		def load(*args)
			method_missing(:load, *args)
		end
	end

	class FileItem
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class FileMetaInfoGroup
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class FileMetaInfoItem
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class FileTreeBranch
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class FilterDev
		def open(*args)
			method_missing(:open, *args)
		end
	end

	class HTMLView
		def print(*args)
			method_missing(:print, *args)
		end
	end

	class Icon
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class ImageEffect
		def hash(*args)
			method_missing(:hash, *args)
		end
	end

	class ImageIO
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class ListView
		include Enumerable

		def each
			it = Qt::ListViewItemIterator.new(self)
			while it.current
				yield it.current
				it += 1
			end
		end
	end

	class ListViewItem
		include Enumerable

		def each
			it = Qt::ListViewItemIterator.new(self)
			while it.current
				yield it.current
				it += 1
			end
		end

		def inspect
			str = super
			str.sub!(/>$/, "")
			for i in 0..(listView.columns - 1)
				str << " text%d=%s," % [i, self.text(i)]
			end
			str.sub!(/,?$/, ">")
		end
		
		def pretty_print(pp)
			str = to_s
			str.sub!(/>$/, "")
			for i in 0..(listView.columns - 1)
				str << " text%d=%s," % [i, self.text(i)]
			end
			str.sub!(/,?$/, ">")
			pp.text str
		end
	end

	class MainWindowInterface
		def raise(*args)
			method_missing(:raise, *args)
		end
	end

	class MdiChildView
		def raise(*args)
			method_missing(:raise, *args)
		end
	end

	class MimeType
		def load(*args)
			method_missing(:load, *args)
		end
	end

	class MultiTabBarButton
		def id(*args)
			method_missing(:id, *args)
		end
	end

	class MultipleDrag
		def format(*args)
			method_missing(:format, *args)
		end
	end

	class NamedCommand
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class NewStuff
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class OCRDialog
		def id(*args)
			method_missing(:id, *args)
		end
	end

	class Palette
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class PanelApplet
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class PanelExtension
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class Pixmap
		def load(*args)
			method_missing(:load, *args)
		end
	end

	class PixmapEffect
		def hash(*args)
			method_missing(:hash, *args)
		end
	end

	class PluginInfo
		def load(*args)
			method_missing(:load, *args)
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end

	class PluginSelector
		def load(*args)
			method_missing(:load, *args)
		end
	end

	class PopupFrame
		def exec(*args)
			method_missing(:exec, *args)
		end
	end

	class PrintAction
		def print(*args)
			method_missing(:print, *args)
		end
	end

	class Printer
		def abort(*args)
			method_missing(:abort, *args)
		end
	end

	class Progress
		def format(*args)
			method_missing(:format, *args)
		end
	end

	class ProtocolInfo
		def exec(*args)
			method_missing(:exec, *args)
		end

		def load(*args)
			method_missing(:load, *args)
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end

	class Pty
		def open(*args)
			method_missing(:open, *args)
		end
	end

	class Run
		def abort(*args)
			method_missing(:abort, *args)
		end
	end

	class SSLCertDlgRet
		def send(*args)
			method_missing(:send, *args)
		end
	end

	class SSLPKCS12
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class SSLPKCS7
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class SSLSettings
		def load(*args)
			method_missing(:load, *args)
		end
	end

	class SaveFile
		def abort(*args)
			method_missing(:abort, *args)
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ScanDialog
		def id(*args)
			method_missing(:id, *args)
		end
	end
	
	class Service
		def inspect
			str = super
			str.sub(/>$/, " library=%s, type=%s, name=%s>" % [library.inspect, type.inspect, name.inspect])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n library=%s,\n type=%s,\n name=%s>" % [library.inspect, type.inspect, name.inspect])
		end

		def exec(*args)
			method_missing(:exec, *args)
		end

		def load(*args)
			method_missing(:load, *args)
		end

		def name(*args)
			method_missing(:name, *args)
		end

		def type(*args)
			method_missing(:type, *args)
		end
	end

	class ServiceGroup
		def load(*args)
			method_missing(:load, *args)
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ServiceSeparator
		def load(*args)
			method_missing(:load, *args)
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ServiceType
		def load(*args)
			method_missing(:load, *args)
		end

		def name(*args)
			method_missing(:name, *args)
		end
	end

	class Socks
		def select(*args)
			method_missing(:select, *args)
		end

		def send(*args)
			method_missing(:send, *args)
		end
	end

	class StdAccel
		def name(*args)
			method_missing(:name, *args)
		end

		def open(*args)
			method_missing(:open, *args)
		end

		def print(*args)
			method_missing(:print, *args)
		end
	end

	class StdAccel::ShortcutList
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class StdAction
		def name(*args)
			method_missing(:name, *args)
		end

		def open(*args)
			method_missing(:open, *args)
		end

		def print(*args)
			method_missing(:print, *args)
		end
	end

	class StdGuiItem
		def open(*args)
			method_missing(:open, *args)
		end

		def print(*args)
			method_missing(:print, *args)
		end

		def test(*args)
			method_missing(:test, *args)
		end
	end

	class TempDir
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class TempFile
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ToolBarButton
		def id(*args)
			method_missing(:id, *args)
		end
	end

	class UniqueApplication
		def initialize(*k)
			super
			$kapp = self
		end
		
		# Delete the underlying C++ instance after exec returns
		# Otherwise, rb_gc_call_finalizer_at_exit() can delete
		# stuff that KDE::Application still needs for its cleanup.
		def exec
			method_missing(:exec)
			self.dispose
			Qt::Internal.application_terminated = true
		end
	end

	class URIFilterPlugin
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class URL
		def inspect
			str = super
			str.sub(/>$/, " url=%s, protocol=%s, host=%s, port=%d>" % [url.inspect, protocol.inspect, host.inspect, port])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n url=%s,\n protocol=%s,\n host=%s,\n port=%d>" % [url.inspect, protocol.inspect, host.inspect, port])
		end

		def split(*args)
			method_missing(:split, *args)
		end
	end

	class URLDrag
		def format(*args)
			method_missing(:format, *args)
		end
	end

	class VMAllocator
		def allocate(*args)
			method_missing(:allocate, *args)
		end
	end

	class WindowInfo
		def display(*args)
			method_missing(:display, *args)
		end
	end

end

module DOM
	class Attr
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class DOMString
		def split(*args)
			method_missing(:split, *args)
		end
	end

	class DocumentType
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class Event
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLAnchorElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLAnchorElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLAppletElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLButtonElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLButtonElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLElement
		def id(*args)
			method_missing(:id, *args)
		end
	end

	class HTMLFormElement
		def method(*args)
			method_missing(:method, *args)
		end
	end

	class HTMLFormElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLFrameElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLIFrameElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLImageElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLInputElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLInputElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLLIElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLLinkElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLMapElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLMetaElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLOListElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLObjectElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLObjectElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLParamElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLParamElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLScriptElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLSelectElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLSelectElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLStyleElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLTextAreaElement
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class HTMLTextAreaElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class HTMLUListElement
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class StyleSheet
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class CSSRule
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class Document
		def abort(*args)
			method_missing(:abort, *args)
		end
	end

	class Document
		def load(*args)
			method_missing(:load, *args)
		end
	end

	class HTMLDocument
		def open(*args)
			method_missing(:open, *args)
		end
	end

	class HTMLInputElement
		def select(*args)
			method_missing(:select, *args)
		end
	end

	class HTMLTextAreaElement
		def select(*args)
			method_missing(:select, *args)
		end
	end
end # DOM

module KIO
	class Connection
		def send(*args)
			method_missing(:send, *args)
		end
	end

	class NetRC::AutoLogin
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class Slave
		def send(*args)
			method_missing(:send, *args)
		end
	end

	class SlaveBase
		def exit(*args)
			method_missing(:exit, *args)
		end
	end
end # KIO

module KNS
	class Engine
		def type(*args)
			method_missing(:type, *args)
		end
	end

	class Entry
		def name(*args)
			method_missing(:name, *args)
		end

		def type(*args)
			method_missing(:type, *args)
		end
	end

	class Provider
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ProviderLoader
		def load(*args)
			method_missing(:load, *args)
		end
	end
end # KNS

module KParts
	class Event
		def test(*args)
			method_missing(:test, *args)
		end
	end

	class GUIActivateEvent
		def test(*args)
			method_missing(:test, *args)
		end
	end

	class OpenURLEvent
		def test(*args)
			method_missing(:test, *args)
		end
	end

	class PartActivateEvent
		def test(*args)
			method_missing(:test, *args)
		end
	end

	class PartSelectEvent
		def test(*args)
			method_missing(:test, *args)
		end
	end
end # KParts

module Win
	class Win::WindowInfo
		def name(*args)
			method_missing(:name, *args)
		end
	end
end

class Object
	def RESTORE(klass)
		n = 1
		while MainWindow.canBeRestored(n)
			klass.new.restore(n)
			n += 1
		end
	end

	def I18N_NOOP(x) x end
	def I18N_NOOP2(comment, x) x end
end

class Qt::Base
	def self.k_dcop_signals(*signal_list)
		meta = KDE::DCOPMeta[self.name] || KDE::DCOPMetaInfo.new(self)
		meta.add_signals(signal_list)
		meta.changed = true
	end

	def self.k_dcop(*slot_list)
		meta = KDE::DCOPMeta[self.name] || KDE::DCOPMetaInfo.new(self)
		meta.add_slots(slot_list)
		meta.changed = true
	end
end
