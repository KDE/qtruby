=begin
/***************************************************************************
                          qtruby.rb  -  description
                             -------------------
    begin                : Fri Jul 4 2003
    copyright            : (C) 2003-2005 by Richard Dale
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

module Qt
	module DebugLevel
		Off, Minimal, High, Extensive = 0, 1, 2, 3
	end

	module QtDebugChannel 
		QTDB_NONE = 0x00
		QTDB_AMBIGUOUS = 0x01
		QTDB_METHOD_MISSING = 0x02
		QTDB_CALLS = 0x04
		QTDB_GC = 0x08
		QTDB_VIRTUAL = 0x10
		QTDB_VERBOSE = 0x20
		QTDB_ALL = QTDB_VERBOSE | QTDB_VIRTUAL | QTDB_GC | QTDB_CALLS | QTDB_METHOD_MISSING | QTDB_AMBIGUOUS
	end

	@@debug_level = DebugLevel::Off
	def Qt.debug_level=(level)
		@@debug_level = level
		Internal::setDebug Qt::QtDebugChannel::QTDB_ALL if level >= DebugLevel::Extensive
	end

	def Qt.debug_level
		@@debug_level
	end
		
	class Base
		def self.signals(*signal_list)
			meta = Qt::Meta[self.name] || Qt::MetaInfo.new(self)
			meta.add_signals(signal_list)
			meta.changed = true
		end
	
		def self.slots(*slot_list)
			meta = Qt::Meta[self.name] || Qt::MetaInfo.new(self)
			meta.add_slots(slot_list)
			meta.changed = true
		end

		def **(a)
			return Qt::**(self, a)
		end
		def +(a)
			return Qt::+(self, a)
		end
		def ~(a)
			return Qt::~(self, a)
		end
		def -@()
			return Qt::-(self)
		end
		def -(a)
			return Qt::-(self, a)
		end
		def *(a)
			return Qt::*(self, a)
		end
		def /(a)
			return Qt::/(self, a)
		end
		def %(a)
			return Qt::%(self, a)
		end
		def >>(a)
			return Qt::>>(self, a)
		end
		def <<(a)
			return Qt::<<(self, a)
		end
		def &(a)
			return Qt::&(self, a)
		end
		def ^(a)
			return Qt::^(self, a)
		end
		def |(a)
			return Qt::|(self, a)
		end

#		Module has '<', '<=', '>' and '>=' operator instance methods, so pretend they
#		don't exist by calling method_missing() explicitely
		def <(a)
			begin
				Qt::method_missing(:<, self, a)
			rescue
				super(a)
			end
		end

		def <=(a)
			begin
				Qt::method_missing(:<=, self, a)
			rescue
				super(a)
			end
		end

		def >(a)
			begin
				Qt::method_missing(:>, self, a)
			rescue
				super(a)
			end
		end

		def >=(a)
			begin
				Qt::method_missing(:>=, self, a)
			rescue
				super(a)
			end
		end

#		Object has a '==' operator instance method, so pretend it
#		don't exist by calling method_missing() explicitely
		def ==(a)
			begin
				Qt::method_missing(:==, self, a)
			rescue
				super(a)
			end
		end


		def methods(regular=true)
			if !regular
				return singleton_methods
			end
	
			qt_methods(super, 0x0)
		end
	
		def protected_methods
			# From smoke.h, Smoke::mf_protected 0x80
			qt_methods(super, 0x80)
		end
	
		def public_methods
			methods
		end
	
		def singleton_methods
			# From smoke.h, Smoke::mf_static 0x01
			qt_methods(super, 0x01)
		end
	
		private
		def qt_methods(meths, flags)
			ids = []
			# These methods are all defined in Qt::Base, even if they aren't supported by a particular
			# subclass, so remove them to avoid confusion
			meths -= ["%", "&", "*", "**", "+", "-", "-@", "/", "<", "<<", "<=", ">", ">=", ">>", "|", "~", "^"]
			classid = Qt::Internal::idInstance(self)
			Qt::Internal::getAllParents(classid, ids)
			ids << classid
			ids.each { |c| Qt::Internal::findAllMethodNames(meths, c, flags) }
			return meths.uniq
		end
	end # Qt::Base
	
	# Delete the underlying C++ instance after exec returns
	# Otherwise, rb_gc_call_finalizer_at_exit() can delete
	# stuff that Qt::Application still needs for its cleanup.
	
	class Application < Qt::Base
		def exec
			super
			self.dispose
			Qt::Internal.application_terminated = true
		end
	end
	
	class ByteArray < Qt::Base
		def to_s
			return constData()
		end

		def to_i
			return toInt()
		end

		def to_f
			return toDouble()
		end
	end
	
	class Color < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " %s>" % name)
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, " %s>" % name)
		end
	end
	
	class Connection < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " memberName=%s, memberType=%s, object=%s>" %
				[memberName.inspect, memberType == 1 ? "SLOT" : "SIGNAL", object.inspect] )
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n memberName=%s,\n memberType=%s,\n object=%s>" %
				[memberName.inspect, memberType == 1 ? "SLOT" : "SIGNAL", object.inspect] )
		end
	end
	
	class Cursor < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " shape=%d>" % shape)
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, " shape=%d>" % shape)
		end
	end
	
	class Font < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " family=%s, pointSize=%d, weight=%d, italic=%s, bold=%s, underline=%s, strikeOut=%s>" % 
			[family.inspect, pointSize, weight, italic, bold, underline, strikeOut])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n family=%s,\n pointSize=%d,\n weight=%d,\n italic=%s,\n bold=%s,\n underline=%s,\n strikeOut=%s>" % 
			[family.inspect, pointSize, weight, italic, bold, underline, strikeOut])
		end
	end
	
	class LCDNumber < Qt::Base
		def display(item)
			method_missing(:display, item)
		end
	end
	
	class Point < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " x=%d, y=%d>" % [x, y])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n x=%d,\n y=%d>" % [x, y])
		end
	end
	
	class PointF < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " x=%f, y=%f>" % [x, y])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n x=%f,\n y=%f>" % [x, y])
		end
	end

	class MetaMethod < Qt::Base
		# Oops, name clash with the Signal module so hard code
		# this value rather than get it from the Smoke runtime
		Signal = 1
	end

	class MetaObject < Qt::Base
		alias_method :_method, :method

		def method(arg)
			if arg.kind_of? Symbol
				_method(arg)
			else
				method_missing(:method, arg)
			end
		end

		# Add two methods, 'slotNames()' and 'signalNames()' from
		# Qt3, as they are very useful when debugging
		def slotNames(inherits = false)
			res = []
			if inherits
				(0...methodCount()).each do |m| 
					if method(m).methodType == Qt::MetaMethod::Slot 
						res.push method(m).signature
					end
				end
			else
				(methodOffset()...methodCount()).each do |m| 
					if method(m).methodType == Qt::MetaMethod::Slot 
						res.push method(m).signature
					end
				end
			end
			return res
		end

		def signalNames(inherits = false)
			res = []
			if inherits
				(0...methodCount()).each do |m| 
					if method(m).methodType == Qt::MetaMethod::Signal 
						res.push method(m).signature
					end
				end
			else
				(methodOffset()...methodCount()).each do |m| 
					if method(m).methodType == Qt::MetaMethod::Signal 
						res.push method(m).signature
					end
				end
			end
			return res
		end

		def inspect
			str = super
			str.sub!(/>$/, "")
			str << " className=%s," % className
			str << " signalNames=Array (%d element(s))," % signalNames.length unless signalNames.length == 0
			str << " slotNames=Array (%d element(s))," % slotNames.length unless slotNames.length == 0
			str << " superClass=%s," % superClass.inspect unless superClass == nil
			str.chop!
			str << ">"
		end
		
		def pretty_print(pp)
			str = to_s
			str.sub!(/>$/, "")
			str << "\n className=%s," % className
			str << "\n signalNames=Array (%d element(s))," % signalNames.length unless signalNames.length == 0
			str << "\n slotNames=Array (%d element(s))," % slotNames.length unless slotNames.length == 0
			str << "\n superClass=%s," % superClass.inspect unless superClass == nil
			str << "\n methodCount=%d," % methodCount
			str << "\n methodOffset=%d," % methodOffset
			str << "\n propertyCount=%d," % propertyCount
			str << "\n propertyOffset=%d," % propertyOffset
			str << "\n enumeratorCount=%d," % enumeratorCount
			str << "\n enumeratorOffset=%d," % enumeratorOffset
			str.chop!
			str << ">"
			pp.text str
		end
	end
	
	class Line < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " x1=%d, y1=%d, x2=%d, y2=%d>" % [x1, y1, x2, y2])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n x1=%d,\n y1=%d,\n x2=%d,\n y2=%d>" % [x1, y1, x2, y2])
		end
	end
	
	class LineF < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " x1=%f, y1=%f, x2=%f, y2=%f>" % [x1, y1, x2, y2])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n x1=%f,\n y1=%f,\n x2=%f,\n y2=%f>" % [x1, y1, x2, y2])
		end
	end
	
	class Rect < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " left=%d, right=%d, top=%d, bottom=%d>" % [left, right, top, bottom])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n left=%d,\n right=%d,\n top=%d,\n bottom=%d>" % [left, right, top, bottom])
		end
	end
	
	class RectF < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " left=%f, right=%f, top=%f, bottom=%f>" % [left, right, top, bottom])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n left=%f,\n right=%f,\n top=%f,\n bottom=%f>" % [left, right, top, bottom])
		end
	end
	
	class Size < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " width=%d, height=%d>" % [width, height])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n width=%d,\n height=%d>" % [width, height])
		end
	end
	
	class SizeF < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " width=%f, height=%f>" % [width, height])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n width=%f,\n height=%f>" % [width, height])
		end
	end
	
	class SizePolicy < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " horData=%d, verData=%d>" % [horData, verData])
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, "\n horData=%d,\n verData=%d>" % [horData, verData])
		end
	end
	
	class Date < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " %s>" % toString)
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, " %s>" % toString)
		end
	end
	
	class DateTime < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " %s>" % toString)
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, " %s>" % toString)
		end
	end
	
	class Time < Qt::Base
		def inspect
			str = super
			str.sub(/>$/, " %s>" % toString)
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, " %s>" % toString)
		end
	end
	
	class Variant < Qt::Base
		def to_a
			return toStringList()
		end

		def to_f
			return toDouble()
		end

		def to_i
			return toInt()
		end

		def to_int
			return toInt()
		end

		def inspect
			str = super
			str.sub(/>$/, " typeName=%s>" % typeName)
		end
		
		def pretty_print(pp)
			str = to_s
			pp.text str.sub(/>$/, " typeName=%s>" % typeName)
		end
	end
	
	# Provides a mutable numeric class for passing to methods with
	# C++ 'int*' or 'int&' arg types
	class Integer
		attr_accessor :value
		def initialize(n=0) @value = n end
		
		def +(n) 
			return Integer.new(@value + n.to_i) 
		end
		def -(n) 
			return Integer.new(@value - n.to_i)
		end
		def *(n) 
			return Integer.new(@value * n.to_i)
		end
		def /(n) 
			return Integer.new(@value / n.to_i)
		end
		def %(n) 
			return Integer.new(@value % n.to_i)
		end
		def **(n) 
			return Integer.new(@value ** n.to_i)
		end
		
		def |(n) 
			return Integer.new(@value | n.to_i)
		end
		def &(n) 
			return Integer.new(@value & n.to_i)
		end
		def ^(n) 
			return Integer.new(@value ^ n.to_i)
		end
		def <<(n) 
			return Integer.new(@value << n.to_i)
		end
		def >>(n) 
			return Integer.new(@value >> n.to_i)
		end
		def >(n) 
			return @value > n.to_i
		end
		def >=(n) 
			return @value >= n.to_i
		end
		def <(n) 
			return @value < n.to_i
		end
		def <=(n) 
			return @value <= n.to_i
		end
		
		def <=>(n)
			if @value < n.to_i
				return -1
			elsif @value > n.to_i
				return 1
			else
				return 0
			end
		end
		
		def to_f() return @value.to_f end
		def to_i() return @value.to_i end
		def to_s() return @value.to_s end
		
		def coerce(n)
			[n, @value]
		end
	end
	
	# If a C++ enum was converted to an ordinary ruby Integer, the
	# name of the type is lost. The enum type name is needed for overloaded
	# method resolution when two methods differ only by an enum type.
	class Enum
		attr_accessor :type, :value
		def initialize(n, type)
			super() 
			@value = n 
			@type = type
		end
		
		def +(n) 
			return @value + n.to_i
		end
		def -(n) 
			return @value - n.to_i
		end
		def *(n) 
			return @value * n.to_i
		end
		def /(n) 
			return @value / n.to_i
		end
		def %(n) 
			return @value % n.to_i
		end
		def **(n) 
			return @value ** n.to_i
		end
		
		def |(n) 
			return Enum.new(@value | n.to_i, @type)
		end
		def &(n) 
			return Enum.new(@value & n.to_i, @type)
		end
		def ^(n) 
			return Enum.new(@value ^ n.to_i, @type)
		end
		def ~() 
			return ~ @value
		end
		def <(n) 
			return @value < n.to_i
		end
		def <=(n) 
			return @value <= n.to_i
		end
		def >(n) 
			return @value > n.to_i
		end
		def >=(n) 
			return @value >= n.to_i
		end
		def <<(n) 
			return Enum.new(@value << n.to_i, @type)
		end
		def >>(n) 
			return Enum.new(@value >> n.to_i, @type)
		end
		
		def ==(n) return @value == n.to_i end
		def to_i() return @value end

		def to_f() return @value.to_f end
		def to_s() return @value.to_s end
		
		def coerce(n)
			[n, @value]
		end
		
		def inspect
			to_s
		end

		def pretty_print(pp)
			pp.text "#<%s:0x%8.8x @type=%s, @value=%d>" % [self.class.name, object_id, type, value]
		end
	end
	
	# Provides a mutable boolean class for passing to methods with
	# C++ 'bool*' or 'bool&' arg types
	class Boolean
		attr_accessor :value
		def initialize(b=false) @value = b end
		def nil? 
			return !@value 
		end
	end
	
	module Internal
		@@classes   = {}
		@@cpp_names = {}
		@@idclass   = []

		def Internal.normalize_classname(classname)
			if classname =~ /^Qext/
				now = classname.sub(/^Qext(?=[A-Z])/,'Qext::')
			elsif classname =~ /^Q/
				now = classname.sub(/^Q(?=[A-Z])/,'Qt::')
			elsif classname =~ /^(KConfigSkeleton|KWin)::/
				now = classname.sub(/^K?(?=[A-Z])/,'KDE::')
			elsif classname !~ /::/
				now = classname.sub(/^K?(?=[A-Z])/,'KDE::')
			else
				now = classname
			end
			# puts "normalize_classname = was::#{classname}, now::#{now}"
			now
		end

		def Internal.init_class(c)
			classname = Qt::Internal::normalize_classname(c)
			classId = Qt::Internal.idClass(c)
			insert_pclassid(classname, classId)
			@@idclass[classId] = classname
			@@cpp_names[classname] = c
			klass = isQObject(classId) ? create_qobject_class(classname) \
                                                   : create_qt_class(classname)
			@@classes[classname] = klass unless klass.nil?
		end

		def Internal.debug_level
			Qt.debug_level
		end

		def Internal.checkarg(argtype, typename)
			puts "      #{typename} (#{argtype})" if debug_level >= DebugLevel::High
			if argtype == 'i'
				if typename =~ /^int&?$|^signed int&?$|^signed$|^qint32&?$/
					return 1
				elsif typename =~ /^(?:short|ushort|unsigned short int|uchar|uint|long|ulong|unsigned long int|unsigned|float|double)$/
					return 0
				elsif typename =~ /^(quint|qint|qulong|qlong|qreal)/
					return 0
				else 
					t = typename.sub(/^const\s+/, '')
					t.sub!(/[&*]$/, '')
					if isEnum(t)
						return 0
					end
				end
			elsif argtype == 'n'
				if typename =~ /^double$|^qreal$/
					return 2
				elsif typename =~ /^float$/
					return 1
				elsif typename =~ /^int&?$/
					return 0
				elsif typename =~ /^(?:short|ushort|uint|long|ulong|signed|unsigned|float|double)$/
					return 0
				else 
					t = typename.sub(/^const\s+/, '')
					t.sub!(/[&*]$/, '')
					if isEnum(t)
						return 0
					end
				end
			elsif argtype == 'B'
				if typename =~ /^(?:bool)[*&]?$/
					return 0
				end
			elsif argtype == 's'
				if typename =~ /^(const )?((QChar)[*&]?)$/
					return 1
				elsif typename =~ /^(?:u?char\*|const u?char\*|(?:const )?(Q(C?)String)[*&]?)$/
					qstring = !$1.nil?
					c = ("C" == $2)
					return c ? 2 : (qstring ? 3 : 0)
				end
			elsif argtype == 'a'
				# FIXME: shouldn't be hardcoded. Installed handlers should tell what ruby type they expect.
				if typename =~ /^(?:
						const\ QCOORD\*|
						(?:const\ )?
						(?:
						    QStringList[\*&]?|
						    QValueList<int>[\*&]?|
						    QRgb\*|
						    char\*\*
						)
					        )$/x
					return 0
				end
			elsif argtype == 'u'
				# Give nil matched against string types a higher score than anything else
				if typename =~ /^(?:u?char\*|const u?char\*|(?:const )?((Q(C?)String))[*&]?)$/
					return 1
				# Numerics will give a runtime conversion error, so they fail the match
				elsif typename =~ /^(?:short|ushort|uint|long|ulong|signed|unsigned|int)$/
					return -99
				else
					return 0
				end
			elsif argtype == 'U'
				if typename =~ /QStringList/
					return 1
				else
					return 0
				end
			else
				t = typename.sub(/^const\s+/, '')
				t.sub!(/[&*]$/, '')
				if argtype == t
					return 1
				elsif classIsa(argtype, t)
					return 0
				elsif isEnum(argtype) and 
						(t =~ /int|qint32|uint|quint32|long|ulong/ or isEnum(t))
					return 0
				end
			end
			return -99
		end

		def Internal.find_class(classname)
			# puts @@classes.keys.sort.join "\n"
			@@classes[classname]
		end
		
		# Runs the initializer as far as allocating the Qt C++ instance.
		# Then use a throw to jump back to here with the C++ instance 
		# wrapped in a new ruby variable of type T_DATA
		def Internal.try_initialize(instance, *args)
			initializer = instance.method(:initialize)
			catch "newqt" do
				initializer.call(*args)
			end
		end
		
        # If a block was passed to the constructor, then
		# run that now. Either run the context of the new instance
		# if no args were passed to the block. Or otherwise,
		# run the block in the context of the arg.
		def Internal.run_initializer_block(instance, block)
			if block.arity == -1
				instance.instance_eval(&block)
			elsif block.arity == 1
				block.call(instance)
			else
				raise ArgumentError, "Wrong number of arguments to block(#{block.arity} for 1)"
			end
		end

		def Internal.do_method_missing(package, method, klass, this, *args)
			if klass.class == Module
				classname = klass.name
			else
				classname = @@cpp_names[klass.name]
				if classname.nil?
					if klass != Object and klass != Qt
						return do_method_missing(package, method, klass.superclass, this, *args)
					else
						return nil
					end
				end
			end
			
			if method == "new"
				method = classname.dup 
				method.gsub!(/^(QTextLayout|KParts|KIO|KNS|DOM|Kontact|Kate|KTextEditor|KConfigSkeleton::ItemEnum|KConfigSkeleton|KWin)::/,"")
			end
			method = "operator" + method.sub("@","") if method !~ /[a-zA-Z]+/
			# Change foobar= to setFoobar()					
			method = 'set' + method[0,1].upcase + method[1,method.length].sub("=", "") if method =~ /.*[^-+%\/|=]=$/

			methods = []
			methods << method.dup
			args.each do |arg|
				if arg.nil?
					# For each nil arg encountered, triple the number of munged method
					# templates, in order to cover all possible types that can match nil
					temp = []
					methods.collect! do |meth| 
						temp << meth + '?' 
						temp << meth + '#'
						meth << '$'
					end
					methods.concat(temp)
				elsif isObject(arg)
					methods.collect! { |meth| meth << '#' }
				elsif arg.kind_of? Array or arg.kind_of? Hash
					methods.collect! { |meth| meth << '?' }
				else
					methods.collect! { |meth| meth << '$' }
				end
			end
			
			methodIds = []
			methods.collect { |meth| methodIds.concat( findMethod(classname, meth) ) }
			
			if method =~ /_/ && methodIds.length == 0
				# If the method name contains underscores, convert to camel case
				# form and try again
				method.gsub!(/(.)_(.)/) {$1 + $2.upcase}
				return do_method_missing(package, method, klass, this, *args)
			end

			if debug_level >= DebugLevel::High
				puts "classname    == #{classname}"
				puts ":: method == #{method}"
				puts "-> methodIds == #{methodIds.inspect}"
				puts "candidate list:"
				prototypes = dumpCandidates(methodIds).split("\n")
				line_len = (prototypes.collect { |p| p.length }).max
				prototypes.zip(methodIds) { 
					|prototype,id| puts "#{prototype.ljust line_len}  (#{id})" 
				}
			end
			
			chosen = nil
			if methodIds.length > 0
				best_match = -1
				methodIds.each do
					|id|
					puts "matching => #{id}" if debug_level >= DebugLevel::High
					current_match = 0
					(0...args.length).each do
						|i|
						current_match += checkarg( getVALUEtype(args[i]), getTypeNameOfArg(id, i) )
					end
					
					# Note that if current_match > best_match, then chosen must be nil
					if current_match > best_match
						best_match = current_match
						chosen = id
					# Multiple matches are an error; the equality test below _cannot_ be commented out.
					# If ambiguous matches occur the problem must be fixed be adjusting the relative
					# ranking of the arg types involved in checkarg().
					elsif current_match == best_match
						chosen = nil
					end
					puts "match => #{id} score: #{current_match}" if debug_level >= DebugLevel::High
				end
					
				puts "Resolved to id: #{chosen}" if !chosen.nil? && debug_level >= DebugLevel::High
			end

			if debug_level >= DebugLevel::Minimal && chosen.nil? && method !~ /^operator/
				id = find_pclassid(normalize_classname(klass.name))
				hash = findAllMethods(id)
				constructor_names = nil
				if method == classname
					puts "No matching constructor found, possibles:\n"
					constructor_names = hash.keys.grep(/^#{classname}/)
				else
					puts "Possible prototypes:"
					constructor_names = hash.keys
				end
				method_ids = hash.values_at(*constructor_names).flatten
				puts dumpCandidates(method_ids)
			end

			puts "setCurrentMethod(#{chosen})" if debug_level >= DebugLevel::High
			setCurrentMethod(chosen) if chosen
			return nil
		end

		def Internal.init_all_classes()
			Qt::Internal::getClassList().each do |c|
				if c == "Qt"
					# Don't change Qt to Qt::t, just leave as is
					@@cpp_names["Qt"] = c
				elsif c != "QInternal"
					Qt::Internal::init_class(c)
				end
			end

			@@classes['Qt::Integer'] = Qt::Integer
			@@classes['Qt::Boolean'] = Qt::Boolean
			@@classes['Qt::Enum'] = Qt::Enum
		end
		
		def Internal.get_qinteger(num)
			return num.value
		end
		
		def Internal.set_qinteger(num, val)
			return num.value = val
		end
		
		def Internal.create_qenum(num, type)
			return Qt::Enum.new(num, type)
		end
		
		def Internal.get_qenum_type(e)
			return e.type
		end
		
		def Internal.get_qboolean(b)
			return b.value
		end
		
		def Internal.set_qboolean(b, val)
			return b.value = val
		end

		def Internal.getAllParents(class_id, res)
			getIsa(class_id).each do |s|
				c = idClass(s)
				res << c
				getAllParents(c, res)
			end
		end
	
		def Internal.getSignalNames(klass)
			meta = Meta[klass.name] || MetaInfo.new(klass)
			signal_names = []
			meta.get_signals.each do |signal|
				signal_names.push signal.name
			end
			return signal_names 
		end
	
		def Internal.signalInfo(qobject, signal_name)
			signals = Meta[qobject.class.name].get_signals
			signals.each_with_index do |signal, i|
				if signal.name == signal_name
					return [signal.full_name, i]
				end
			end
		end
	
		def Internal.signalAt(qobject, index)
			classname = qobject.class.name
			Meta[classname].get_signals[index].full_name
		end
	
		def Internal.slotAt(qobject, index)
			classname = qobject.class.name
			Meta[classname].get_slots[index].full_name
		end
	
		def Internal.getMocArguments(member)
			argStr = member.sub(/.*\(/, '').sub(/\)$/, '')
			args = argStr.scan(/([^,]*<[^>]+>)|([^,]+)/)
			mocargs = allocateMocArguments(args.length)
			args.each_with_index do |arg, i|
				arg = arg.to_s
				a = arg.sub(/^const\s+/, '')
				a = (a =~ /^(bool|int|double|char\*|QString)&?$/) ? $1 : 'ptr'
				valid = setMocType(mocargs, i, arg, a)
			end
			result = []
			result << args.length << mocargs
			result
		end

		#
		# From the enum MethodFlags in qt-copy/src/tools/moc/generator.cpp
		#
		AccessPrivate = 0x00
		AccessProtected = 0x01
		AccessPublic = 0x02
		MethodMethod = 0x00
		MethodSignal = 0x04
		MethodSlot = 0x08
		MethodCompatibility = 0x10
		MethodCloned = 0x20
		MethodScriptable = 0x40
	
		# Keeps a hash of strings against their corresponding offsets
		# within the qt_meta_stringdata sequence of null terminated
		# strings. Returns a proc to get an offset given a string.
		# That proc also adds new strings to the 'data' array, and updates 
		# the corresponding 'pack_str' Array#pack template.
		def Internal.string_table_handler(data, pack_str)
			hsh = {}
			offset = 0
			return lambda do |str|
				if !hsh.has_key? str
					hsh[str] = offset
					data << str
					pack_str << "a*x"
					offset += str.length + 1
				end

				return hsh[str]
			end
		end

		def Internal.makeMetaData(classname, signals, slots)
			# Each entry in 'stringdata' corresponds to a string in the
			# qt_meta_stringdata_<classname> structure.
			# 'pack_string' is used to convert 'stringdata' into the
			# binary sequence of null terminated strings for the metaObject
			stringdata = []
			pack_string = ""
			string_table = string_table_handler(stringdata, pack_string)

			# This is used to create the array of uints that make up the
			# qt_meta_data_<classname> structure in the metaObject
			data = [1, 								# revision
					string_table.call(classname), 	# classname
					0, 0, 							# classinfo
					signals.length + slots.length, 10, 	# methods
					0, 0, 							# properties
					0, 0]							# enums/sets

			signals.each do |entry|
				data.push string_table.call(entry.full_name)				# signature
				data.push string_table.call(entry.full_name.delete("^,"))	# parameters
				data.push string_table.call("")				# type, "" means void
				data.push string_table.call("")				# tag
				data.push MethodSignal | AccessProtected	# flags, always protected for now
			end

			slots.each do |entry|
				data.push string_table.call(entry.full_name)				# signature
				data.push string_table.call(entry.full_name.delete("^,"))	# parameters
				data.push string_table.call("")				# type, "" means void
				data.push string_table.call("")				# tag
				data.push MethodSlot | AccessPublic			# flags, always public for now
			end

			data.push 0		# eod

			return [stringdata.pack(pack_string), data]
		end
		
		def Internal.getMetaObject(qobject)
			meta = Meta[qobject.class.name]
			return nil if meta.nil?
	
			if meta.metaobject.nil? or meta.changed
				signals 			= meta.get_signals
				slots 				= meta.get_slots
				stringdata, data 	= makeMetaData(qobject.class.name, signals, slots)
				meta.metaobject 	= make_metaObject(qobject, stringdata, data)
				meta.changed = false
			end
			
			meta.metaobject
		end
	end # Qt::Internal

	Meta = {}
	
	# An entry for each signal or slot
	# Example 
	#  foobar(QString,bool)
	#  :name is 'foobar'
	#  :full_name is 'foobar(QString,bool)'
	#  :arg_types is 'QString,bool'
	QObjectMember = Struct.new :name, :full_name, :arg_types

	class MetaInfo
		attr_accessor :signals, :slots, :metaobject, :mocargs, :changed
		def initialize(aClass)
			Meta[aClass.name] = self
			@klass = aClass
			@metaobject = nil
			@signals = []
			@slots = []
			@changed = false
		end
		
		def add_signals(signal_list)
			signal_list.each do |signal|
				signal = Qt::MetaObject.normalizedSignature(signal).to_s
				if signal =~ /([^\s]*)\((.*)\)/
					@signals.push QObjectMember.new($1, signal, $2)
				else
					qWarning( "#{@klass.name}: Invalid signal format: '#{signal}'" )
				end
			end
		end
		
		# Return a list of signals, including inherited ones
		def get_signals
			all_signals = []
			current = @klass
			while current != Qt::Base
				meta = Meta[current.name]
				if !meta.nil?
					all_signals.concat meta.signals
				end
				current = current.superclass
			end
			return all_signals
		end
		
		def add_slots(slot_list)
			slot_list.each do |slot|
				slot = Qt::MetaObject.normalizedSignature(slot).to_s
				if slot =~ /([^\s]*)\((.*)\)/
					@slots.push QObjectMember.new($1, slot, $2)
				else
					qWarning( "#{@klass.name}: Invalid slot format: '#{slot}'" )
				end
			end
		end
		
		# Return a list of slots, including inherited ones
		def get_slots
			all_slots = []
			current = @klass
			while current != Qt::Base
				meta = Meta[current.name]
				if !meta.nil?
					all_slots.concat meta.slots
				end
				current = current.superclass
			end
			return all_slots
		end
	end # Qt::MetaInfo

	# These values are from the enum WindowType in qnamespace.h.
	# Some of the names such as 'Qt::Dialog', clash with QtRuby 
	# class names. So add some constants here to use instead,
	# renamed with an ending of 'Type'.
	WidgetType = 0x00000000
	WindowType = 0x00000001
	DialogType = 0x00000002 | WindowType
	SheetType = 0x00000004 | WindowType
	DrawerType = 0x00000006 | WindowType
	PopupType = 0x00000008 | WindowType
	ToolType = 0x0000000a | WindowType
	ToolTipType = 0x0000000c | WindowType
	SplashScreenType = 0x0000000e | WindowType
	DesktopType = 0x00000010 | WindowType
	SubWindowType =  0x00000012
		
end # Qt

class Object
	# The Object.display() method conflicts with display() methods in Qt,
	# so remove it, and other methods with similar problems
	alias_method :_display, :display
	undef_method :display
	
	alias_method :_type, :type
	undef_method :type

	alias_method :_id, :id

	def id(*k)
		if k.length == 0
			_id
		else
			method_missing(:id, *k)
		end
	end

	def SIGNAL(string) ; return "2" + string; end
	def SLOT(string)   ; return "1" + string; end
	def emit(signal)   ; end
end

module Kernel
	# Kernel has a method called open() which takes a String as
	# the first argument. When a call is made to an open() method
	# in the Qt classes, it messes up the method_missing()
	# logic to divert it to the Smoke library. This code
	# fixes that problem by calling the appropriate method based
	# on the type of the first arg.
	alias_method :_open, :open
	
	def open(*k)
		if k.length > 0 and k[0].kind_of? String
			_open(*k)
		else
			method_missing(:open, *k)
		end
	end

	alias_method :_format, :format

	def format(*k)
		if k.length > 0 and k[0].kind_of? String
			_format(*k)
		else
			method_missing(:format, *k)
		end
	end

	alias_method :_exec, :exec

	def exec(*k)
		if k.length > 0 and k[0].kind_of? String
			_exec(*k)
		else
			method_missing(:exec, *k)
		end
	end

	alias_method :_select, :select

	def select(*k)
		if k.length > 1 and k[0].kind_of? Array
			_select(*k)
		else
			method_missing(:select, *k)
		end
	end
end

class Module
	alias_method :_constants, :constants
	alias_method :_instance_methods, :instance_methods
	alias_method :_protected_instance_methods, :protected_instance_methods
	alias_method :_public_instance_methods, :public_instance_methods

	private :_constants, :_instance_methods
	private :_protected_instance_methods, :_public_instance_methods

	def constants
		qt_methods(_constants, 0x10, true)
	end

	def instance_methods(inc_super=true)
		qt_methods(_instance_methods(inc_super), 0x0, inc_super)
	end

	def protected_instance_methods(inc_super=true)
		qt_methods(_protected_instance_methods(inc_super), 0x80, inc_super)
	end

	def public_instance_methods(inc_super=true)
		qt_methods(_public_instance_methods(inc_super), 0x0, inc_super)
	end

	private
	def qt_methods(meths, flags, inc_super=true)
		if !self.kind_of? Class
			return meths
		end

		klass = self
		classid = 0
		loop do
			classid = Qt::Internal::find_pclassid(klass.name)
			break if classid > 0
			
			klass = klass.superclass
			if klass.nil?
				return meths
			end
		end

		# These methods are all defined in Qt::Base, even if they aren't supported by a particular
		# subclass, so remove them to avoid confusion
		meths -= ["%", "&", "*", "**", "+", "-", "-@", "/", "<", "<<", "<=", ">", ">=", ">>", "|", "~", "^"]
		ids = []
		if inc_super
			Qt::Internal::getAllParents(classid, ids)
		end
		ids << classid
		ids.each { |c| Qt::Internal::findAllMethodNames(meths, c, flags) }
		return meths.uniq
	end
end
