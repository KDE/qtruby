=begin
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
                Off, Minimal, High = *(0..2).to_a
        end

        @@debug_level = DebugLevel::Off
        def Qt.debug_level=(level)
                @@debug_level = level
        end
        def Qt.debug_level
                @@debug_level
        end
		
		class Base
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
			def <(a)
				return Qt::<(self, a)
			end
			def <=(a)
				return Qt::<=(self, a)
			end
			def >(a)
				return Qt::>(self, a)
			end
			def >=(a)
				return Qt::>=(self, a)
			end
			def ==(a)
				return Qt::==(self, a)
			end
		end
			
	require 'delegate.rb'
	
	# Allows a QByteArray to be wrapped in an instance which
	# behaves just like a normal ruby String. TODO: override
	# methods which alter the underlying string, in order to
	# sync the QByteArray with the changed string.
	class ByteArray < DelegateClass(String)
		attr_reader :data
		
		def initialize(string, data)
			super(string)
			@data = data
		end
	end
	
	module Internal

		@@classes   = {}
		@@cpp_names = {}
		@@idclass   = []

		def normalize_classname(classname)
			if classname =~ /^Q/
				classname.sub(/^Q(?=[A-Z])/,'Qt::')
			elsif classname =~ /^KParts__/
				classname.sub(/^KParts__/,'KParts::')
			elsif classname =~ /^KIO__/
				classname.sub(/^KIO__/,'KIO::')
			elsif classname =~ /^khtml__/
				classname.sub(/^khtml__/,'khtml::')
			else
				classname.sub(/^K?(?=[A-Z])/,'KDE::')
			end
		end

		def init_class(c)
			classname = normalize_classname(c)
			classId = idClass(c)
			insert_pclassid(classname, classId)
			@@idclass[classId] = classname
			@@cpp_names[classname] = c
			klass = isQObject(classId) ? create_qobject_class(classname) \
                                                   : create_qt_class(classname)
			@@classes[classname] = klass unless klass.nil?
		end

                def debug_level
                        Qt.debug_level
                end

		def checkarg(argtype, typename)
			puts "      #{typename} (#{argtype})" if debug_level >= DebugLevel::High
			if argtype == 'i'
				if typename =~ /^(?:short|ushort|int|uint|long|ulong|signed|unsigned)$/
					return 0
				end
			elsif argtype == 'n'
				if typename =~ /^(?:float|double)$/
					return 0
				end
			elsif argtype == 's'
				if typename =~ /^(?:u?char\*|const u?char\*|(?:const )?((Q(C?)String)|QByteArray)[*&]?)$/
					qstring = !$1.nil?
					c = !$2.nil?
					return c ? 1 : (qstring ? 2 : 0)
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
				if typename =~ /^(?:float|double)$/
					return 0
				end
			elsif argtype == 'U'
				return 0
			else
				t = typename.sub(/^const\s+/, '')
				t.sub!(/[&*]$/, '')
				if classEquals(argtype, t)
					return 1
				elsif classIsa(argtype, t)
					return 0
				end
			end
			return -99
		end

		def find_class(classname)
			@@classes[classname]
		end
		
				# Runs the initializer as far as allocating the Qt C++ instance.
				# Then use the @@current_initializer continuation to jump back to here
		def try_initialize(instance, *args)
			initializer = instance.method(:initialize)
			return callcc {
                                |continuation|
				@@current_initializer = continuation
				initializer.call(*args)
			}
		end

                # continues off here after first stage initialize is complete
		def continue_new_instance(instance)
			@@current_initializer.call(instance)
		end
		
                # If a block was passed to the constructor, then
				# run that now. Either run the context of the new instance
				# if no args were passed to the block. Or otherwise,
				# run the block in the context of the arg.
		def run_initializer_block(instance, block)
			if block.arity == -1
				instance.instance_eval(&block)
			elsif block.arity == 1
				block.call(instance)
			else
				raise ArgumentError, "Wrong number of arguments to block(#{block.arity} for 1)"
			end
		end
		
		def type_char(arg)
		    if arg.nil? or isObject(arg)
			    "#"
		    elsif arg.kind_of? Array or arg.kind_of? Hash
			    "?"
		    else
			    "$"
		    end
		end

		def do_method_missing(package, method, klass, this, *args)
			classname = @@cpp_names[klass.name]
			if classname.nil?
				if klass != Object
					return do_method_missing(package, method, klass.superclass, this, *args)
				else
					return nil
				end
			end
			method = classname.dup if method == "new"
			method = "operator" + method.sub("@","") if method !~ /[a-zA-Z]+/
#			Change foobar= to setFoobar()					
			method = 'set' + method[0,1].upcase + method[1,method.length].sub("=", "") if method =~ /.*[^-+%\/|]=$/

			method_argstr = ""
			args.each {
				|arg| method_argstr << type_char(arg)
			}

			chosen = nil

			methodStr = method + method_argstr
			methodIds = findMethod(classname, methodStr)
			if debug_level >= DebugLevel::High
			    puts "classname    == #{classname}"
			    puts ":: methodStr == #{methodStr}"
			    puts "-> methodIds == #{methodIds.inspect}"
			    puts "candidate list:"
			    prototypes = dumpCandidates(methodIds).split("\n")
			    line_len = (prototypes.collect { |p| p.length }).max
			    prototypes.zip(methodIds) { 
				|prototype,id| puts "#{prototype.ljust line_len}  (#{id})" 
			    }
			end
			
			if methodIds.length == 1 && method !~ /^operator/
				chosen = methodIds[0]
			elsif methodIds.length > 0
				best_match = -1
				methodIds.each do
					|id|
					puts "matching => #{id}" if debug_level >= DebugLevel::High
					current_match = 0
					(0...args.length).each do
						|i|
						current_match += checkarg( getVALUEtype(args[i]), getTypeNameOfArg(id, i) )
					end
					
					if current_match > best_match
						best_match = current_match
						chosen = id
					elsif current_match == best_match
						chosen = nil
					end
					puts "match => #{id} score: #{current_match}" if debug_level >= DebugLevel::High
				end
					
				puts "Resolved to id: #{chosen}" if debug_level >= DebugLevel::High && !chosen.nil?
			end

			if debug_level >= DebugLevel::High && chosen.nil? && method !~ /^operator/
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

		def init_all_classes()
			getClassList().each {
                                |c|
				if c == "Qt"
					# Don't change Qt to Qt::t, just leave as is
					@@cpp_names["Qt"] = c
				elsif c != "QInternal"
					init_class(c)
				end
                        }
			# Special case QByteArray, as it's disguised as a ruby String
			# and not in the public api.
			@@classes['Qt::ByteArray'] = Qt::ByteArray.class
		end
		
		def create_qbytearray(string, data)
			return Qt::ByteArray.new(string, data)
		end
		
		def get_qbytearray(string)
			return string.data
		end
	end

	Meta = {}

	class MetaInfo
		attr_accessor :signals, :slots, :metaobject, :mocargs, :changed
		def initialize(aClass)
			Meta[aClass.name] = self
			@metaobject = nil
			@signals = []
			@slots = []
			@changed = false
		end
	end

	def hasMembers(aClass)
		classname = aClass.name if aClass.is_a? Module
		meta = Meta[classname]
		return !meta.nil? && (meta.signals.length > 0 or meta.slots.length > 0)
	end

	def getAllParents(class_id, res)
		getIsa(class_id).each {
			|s|
			c = idClass(s)
			res << c
			getAllParents(c, res)
		}
	end

	def getSignalNames(aClass)
		classname = aClass.name if aClass.is_a? Module
		signalNames = []
		signals = Meta[classname].signals
                return [] if signals.nil?
                signals.each {
                        |signal| signalNames << signal.sub(/\(.*/, '')
                }
		signalNames
	end

	def signalInfo(qobject, signalName)
		classname = qobject.class.name if qobject.class.is_a? Module
		signals = Meta[classname].signals
                signals.each_with_index {
                        |signal, i|
                        if signal.sub(/\(.*/, '') == signalName
                        	return [signal, i]
                        end
                }
	end

	def signalAt(qobject, index)
		classname = qobject.class.name if qobject.class.is_a? Module
		Meta[classname].signals[index]
	end

	def slotAt(qobject, index)
		classname = qobject.class.name if qobject.class.is_a? Module
		Meta[classname].slots[index]
	end

	def getMocArguments(member)
		argStr = member.sub(/.*\(/, '').sub!(/\)$/, '')
		args = argStr.scan(/[^, ]+/)
		mocargs = allocateMocArguments(args.length)
                args.each_with_index {
                        |arg, i|
			a = arg.sub(/^const\s+/, '')
	    	        a = (a =~ /^(bool|int|double|char\*|QString)&?$/) ? $1 : 'ptr'
			valid = setMocType(mocargs, i, arg, a)
                }
		result = []
		result << args.length << mocargs
		result
	end

	def makeMetaData(data)
		return nil if data.nil?
		tbl = []
		data.each {
			|entry|
			name = entry.sub(/\(.*/, '')
			argStr = entry.sub(/.*\(/, '')
			argStr.sub!(/\)$/, '')
			params = []
			args = argStr.scan(/[^,]+/)
			args.each {
				|arg|
				name = '' # umm.. is this the aim?, well. it works. soo... ;-)
				param = make_QUParameter(name, arg, 0, 1)
                                params << param
			}
			method = make_QUMethod(name, params)
			tbl << make_QMetaData(entry, method)
		}
		make_QMetaData_tbl(tbl)
	end
	
	def getMetaObject(qobject)
		meta = Meta[qobject.class.name]
		return nil if meta.nil?

		if meta.metaobject.nil? or meta.changed
			slotTable       = makeMetaData(meta.slots)
			signalTable     = makeMetaData(meta.signals)
			meta.metaobject = make_metaObject(qobject.class.name, 
							  qobject.staticMetaObject(),
							  slotTable, 
							  meta.slots.length,
							  signalTable, 
							  meta.signals.length)
			meta.changed = false
		end
		
		meta.metaobject
	end

	IO_Direct     = 0x0100
	IO_Sequential = 0x0200
	IO_Combined   = 0x0300
	IO_TypeMask   = 0x0f00
	IO_Raw        = 0x0040
	IO_Async      = 0x0080
	IO_ReadOnly   = 0x0001
	IO_WriteOnly  = 0x0002
	IO_ReadWrite  = 0x0003
	IO_Append     = 0x0004
	IO_Truncate   = 0x0008
	IO_Translate  = 0x0010
	IO_ModeMask   = 0x00ff
	IO_Open       = 0x1000
	IO_StateMask  = 0xf000
	IO_Ok              = 0
	IO_ReadError       = 1
	IO_WriteError      = 2
	IO_FatalError      = 3
	IO_ResourceError   = 4       
	IO_OpenError       = 5
	IO_ConnectError    = 5
	IO_AbortError      = 6
	IO_TimeOutError    = 7
	IO_UnspecifiedError= 8
		
end

class Object
	# The Object.display() method conflicts with display() methods in Qt,
	# so remove it..
	undef_method :display
	def SIGNAL(string) ; return "2" + string; end
	def SLOT(string)   ; return "1" + string; end
	def emit(signal)   ; end
end

class Module
	include Qt

	def signals(*signal_list)
		meta = Meta[self.name] || MetaInfo.new(self)
		meta.signals += signal_list
		meta.changed = true
	end

	def slots(*slot_list)
		meta = Meta[self.name] || MetaInfo.new(self)
		meta.slots += slot_list
		meta.changed = true
	end
end
