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
	class Application
		def initialize(*k)
			super
			$kapp = self
			$qApp = self
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
	
	class UniqueApplication
		def initialize(*k)
			super
			$kapp = self
		end
		
		def exec
			method_missing(:exec)
			self.dispose
			Qt::Internal.application_terminated = true
		end
	end
	
	class AboutData
		def initialize(*args)
			sargs = []
			for i in 0...args.length do
				if [0, 1, 3, 8, 9].include?(i) && (args[i].kind_of?(String) || args[i].nil?)
					sargs << Qt::ByteArray.new(args[i])
				else
					sargs << args[i]
				end
            end

			super(*sargs)
		end

		def addAuthor(*args)
			sargs = []
			for i in 0...args.length do
				if [2, 3].include?(i) && (args[i].kind_of?(String) || args[i].nil?)
					sargs << Qt::ByteArray.new(args[i])
				else
					sargs << args[i]
				end
            end

			super(*sargs)
		end

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

	class AboutLicense
		def name(*args)
			method_missing(:name, *args)
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
	end

	class ArchiveEntry
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class CmdLineArgs < Qt::Base
		include Enumerable

		def CmdLineArgs.init(*k)
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

		def each
			i = 0
			while i < count do
				yield arg(i)
				i += 1
			end
		end

		def length
			count
		end

		def size
			count
		end

		# Allows args["formfactor"] as an alternative to args.getOption("formfactor"),
		# and args[1] as an alternative to args.arg(1)
		def [](arg)
			if arg.kind_of?(String)
				getOption(arg)
			elsif arg.kind_of?(Integer)
				arg(arg)
			else
				nil
			end
		end

		def set?(arg)
			isSet(arg)
		end

		def isSet(arg)
			super(arg.kind_of?(String) ? Qt::ByteArray.new(arg) : arg)
		end

		def getOption(arg)
			super(arg.kind_of?(String) ? Qt::ByteArray.new(arg) : arg)
		end

		def getOptionList(arg)
			super(arg.kind_of?(String) ? Qt::ByteArray.new(arg) : arg)
		end
	end

	class CmdLineOptions
		def add(*args)
			sargs = []
			for i in 0...args.length do
				if [0, 2].include?(i) && (args[i].kind_of?(String) || args[i].nil?)
					sargs << Qt::ByteArray.new(args[i])
				else
					sargs << args[i]
				end
            end
			super(*sargs)
		end
	end

	class ColorCollection
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ComponentData
		def initialize(*args)
			sargs = []
			for i in 0...args.length do
				if [0, 1].include?(i) && (args[i].kind_of?(String) || args[i].nil?)
					sargs << Qt::ByteArray.new(args[i])
				else
					sargs << args[i]
				end
            end

			super(*sargs)
		end
	end

	class Config
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ConfigGroup
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class DoubleNumInput
		def range=(arg)
			if arg.kind_of? Range
				return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
			else
				return super(arg)
			end
		end
	end

	class FileItem
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class IntNumInput
		def range=(arg)
			if arg.kind_of? Range
				return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
			else
				return super(arg)
			end
		end
	end

	class IntValdator
		def range=(arg)
			if arg.kind_of? Range
				return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
			else
				return super(arg)
			end
		end
	end

	class MainWindow
		# A sane alternative to the strange looking C++ template version.
		# There is no need to pass a list of classes, as the Ruby classes
		# of the main windows to be restored are derived from 
		# KDE::MainWindow#classNameOfToplevel
		# Takes an optional block, so a newly created window can be passed
		# into the block. For example:
		#
		# KDE::MainWindow.kRestoreMainWindows {|window| window.caption = "foobar"}
		#
		def self.kRestoreMainWindows(&block)
			n = 1
			while MainWindow.canBeRestored(n)
				className = MainWindow.classNameOfToplevel(n)
				if className =~ /(.*)::(.*)/
					namespace = Object.const_get($1)
					klass = namespace.const_get($2)
				else
					klass = Object.const_get(className)
				end
				obj = klass.new.restore(n)
				if block_given?
					yield obj
				end
				n += 1
			end
		end

		# Yield the numbers of the windows that can be restored
		# in turn
		def self.each_restore
			n = 1
			while MainWindow.canBeRestored(n)
				yield n
				n += 1
			end
		end
	end

	class PageWidgetItem
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class PluginInfo
		def name(*args)
			method_missing(:name, *args)
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
	end
	class ServiceAction
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class StandardAction
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class StandardShortcut
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class StartupInfoData
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class SycocaEntry
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class TempDir
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class TimeZone
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class Url
		def inspect
			str = super
			str.sub(/>$/, " url=%s, protocol=%s>" % [url.inspect, protocol.inspect])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n url=%s,\n protocol=%s>" % [url.inspect, protocol.inspect])
		end
	end

	class UserGroup
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

