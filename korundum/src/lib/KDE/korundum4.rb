=begin
/***************************************************************************
                          Korundum.rb  -  KDE specific ruby runtime, dcop etc.
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003-2004 by Richard Dale
                           (C) 2008 by Arno Rehn
    email                : Richard_Dale@tipitina.demon.co.uk
                           arno@arnorehn.de
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

require 'Qt'

module KDE
	def KDE.i18n(text, *args)
		str = ki18n(text)
		args.each { | arg | str = str.subs(arg) }
		return str.toString
	end

	def KDE.i18nc(context, text, *args)
		str = ki18nc(context, text)
		args.each { | arg | str = str.subs(arg) }
		return str.toString
	end

	def KDE.i18np(singular, plural, *args)
		str = ki18np(singular, plural)
		args.each { | arg | str = str.subs(arg) }
		return str.toString
	end

	def KDE.i18ncp(context, singular, plural, *args)
		str = ki18ncp(context, singular, plural)
		args.each { | arg | str = str.subs(arg) }
		return str.toString
	end

	class Application < Qt::Base
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
	
	class UniqueApplication < Qt::Base
		def initialize(*k)
			super
			$kapp = self
			$qApp = self
		end
		
		def exec
			method_missing(:exec)
			self.dispose
			Qt::Internal.application_terminated = true
		end
	end
	
	class AboutData < Qt::Base
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

	class AboutLicense < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end
	
	class AboutPerson < Qt::Base
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
	
	class AboutTranslator < Qt::Base
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

	class Archive < Qt::Base
		UnknownTime = Qt::Enum.new(-1, "KArchive")
	end

	class ArchiveEntry < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class CmdLineArgs < Qt::Base
		include Enumerable

		def CmdLineArgs.init(*args)
			if args.length > 0
				if args[0].kind_of? Array
					# If init() is passed an array as the first argument, assume it's ARGV.
					# Then convert to a pair of args 'ARGV.length+1, [$0]+ARGV'
					array = args.shift
					if args[0].kind_of? KDE::AboutData
						super(*([array.length+1] + [[$0] + array] + args))
					else
						sargs = []
						for i in 0...args.length do
							if [0, 1, 3].include?(i) && (args[i].kind_of?(String) || args[i].nil?)
								sargs << Qt::ByteArray.new(args[i])
							else
								sargs << args[i]
							end
            			end
						super(*([array.length+1] + [[$0] + array] + sargs))
					end
				elsif args[0].kind_of? KDE::AboutData
					super(1, [$0], args[0])
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

		def count
			method_missing(:count)
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

	class CmdLineOptions < Qt::Base
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

	class ColorCollection < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ComponentData < Qt::Base
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

	class Config < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class ConfigGroup < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end

        def readEntry(*args)
            if args.length < 2
                return super(*args)
            elsif args[1].kind_of?(String) || args[1].kind_of?(Qt::Variant)
                return super(*args)
            else
                return super(args[0], Qt::Variant.new(args[1])).value
            end
        end

		def read_entry(*args)
			readEntry(*args)
		end

		def writeEntry(key, value, pFlags = KDE::ConfigBase::Normal)
			if value.kind_of?(String) || value.kind_of?(Qt::Variant)
				super(key, value, pFlags)
			else
				super(key, Qt::Variant.new(value), pFlags)
			end
		end

		def write_entry(key, value, pFlags = KDE::ConfigBase::Normal)
			writeEntry(key, value, pFlags)
		end
	end

	class Dialog < Qt::Base
		def exec(*args)
			method_missing(:exec, *args)
		end
	end

	class DoubleNumInput < Qt::Base
		def range=(arg)
			if arg.kind_of? Range
				return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
			else
				return super(arg)
			end
		end
	end

	class File < Qt::Base
		File = Qt::Enum.new(1, "KFile::Mode")
	end

	class FileItem < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class IntNumInput < Qt::Base
		def range=(arg)
			if arg.kind_of? Range
				return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
			else
				return super(arg)
			end
		end
	end

	class IntValdator < Qt::Base
		def range=(arg)
			if arg.kind_of? Range
				return super(arg.begin, arg.exclude_end?  ? arg.end - 1 : arg.end)
			else
				return super(arg)
			end
		end
	end

	class Job < Qt::Base
		def exec(*args)
			method_missing(:exec, *args)
		end
	end

	class MainWindow < Qt::Base
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

	class PageDialog < Qt::Base
		def exec(*args)
			method_missing(:exec, *args)
		end
	end

	class PageWidgetItem < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class PluginInfo < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class PopupFrame < Qt::Base
		def exec(*args)
			method_missing(:exec, *args)
		end
	end

	class ProtocolInfo < Qt::Base
		def exec(*args)
			method_missing(:exec, *args)
		end
	end

	class Service < Qt::Base
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
	end

	class ServiceAction < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end

		def exec(*args)
			method_missing(:exec, *args)
		end
	end

	class Shortcut < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " %s>" % toString)
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, " %s>" % toString)
		end
	end

	class StandardAction < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end

		def open(*args)
			method_missing(:name, *args)
		end
	end

	class StandardShortcut < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class StartupInfoData < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class SycocaEntry < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class TempDir < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class TimeZone < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	class Url < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " url=%s, protocol=%s>" % [url.inspect, protocol.inspect])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n url=%s,\n protocol=%s>" % [url.inspect, protocol.inspect])
		end
	end

	class UserGroup < Qt::Base
		def name(*args)
			method_missing(:name, *args)
		end
	end

	module Internal
		def self.init_all_classes
			Qt::Internal::add_normalize_proc(Proc.new do |classname|
				if classname =~ /^KCoreConfigSkeleton::/
					# Make classes under KCoreConfigSkeleton appear under KDE::ConfigSkeleton
					# in Ruby, as the KCoreConfigSkeleton class isn't really the public api
					now = classname.sub(/KCore/,'KDE::')
				elsif classname =~ /^(KConfigSkeleton|KWin|KDateTime|KTimeZone|KSettings)::/
					now = classname.sub(/^K?(?=[A-Z])/,'KDE::')
				elsif classname =~ /^K/ && classname !~ /::/
					now = classname.sub(/(^K)/, 'KDE::')
				elsif classname =~ /^TerminalInterface/
					now = "KDE::#{classname}"
				end
				now
			end)
			getClassList.each do |c|
				classname = Qt::Internal::normalize_classname(c)
				id = Qt::Internal::findClass(c);
				Qt::Internal::insert_pclassid(classname, id)
				Qt::Internal::cpp_names[classname] = c
				if classname =~ /^KParts/
					m = KParts
				elsif classname =~ /^KIO/
					m = KIO
				elsif classname =~ /^Sonnet/
					m = Sonnet
				elsif classname =~ /^KNS/
					m = KNS
				elsif classname =~ /^KWallet/
					m = KWallet
				else
					m = KDE
				end
				klass = Qt::Internal::isQObject(c) ? Qt::Internal::create_qobject_class(classname, m) \
													: Qt::Internal::create_qt_class(classname, m)
				Qt::Internal::classes[classname] = klass unless klass.nil?
			end
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
