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
		
	module Internal

		@@classes   = {}
		@@cpp_names = {}
		@@idclass   = []

		def normalize_classname(classname)
			classname.sub(/^Q(?=[A-Z])/,'Qt::')
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

		def checkarg(argtype, method, i)
			typename = getTypeNameOfArg(method, i)
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
				if classIsa(argtype, t)
					return 0
				end
			end
			return nil
		end

		def arg_matches?(methodIds, args, i)
			match = {}
			argtype = getVALUEtype(args[i])
                        methodIds.each {
                                |id|
				puts "   #{id}:" if debug_level >= DebugLevel::High
				match_value = checkarg(argtype, id, i)
				match[id] = match_value unless match_value.nil?
                        }
			return match.sort {|a,b| a[1] <=> b[1]}
		end

		def find_class(classname)
			@@classes[classname]
		end

                # 
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
		
                # If a block was passed to the constructor then
				# run that now in the context of the new instance
		def run_initializer_block(instance, block)
			instance.instance_eval(&block)
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
			if classname.nil? && klass != Object
				do_method_missing(package, method, klass.superclass, this, *args)
				return nil
			end
			method = classname.dup if method == "new"
			method = "operator" + method.sub("@","") if method !~ /[a-zA-Z]+/
#			Change foobar= to setFoobar()					
			method = 'set' + method[0,1].upcase + method[1,method.length].sub("=", "") if method =~ /.*=$/
#			Don't convert boolean property foobar? to isFoobar() for now as they can also		
#			be hasFoobar() or just foobar()	in Qt
#			method = 'is' + method[0,1].upcase + method[1,method.length].sub("?", "") if method =~ /.*\?$/

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
			if methodIds.length == 1
				chosen = methodIds[0]
			elsif methodIds.length > 1
				puts "attempting to resolve:" if debug_level >= DebugLevel::High
				remainingIds = methodIds.dup
				matching = nil
				(0...args.length).each {
					|i|
					puts "arg #{i}:" if debug_level >= DebugLevel::High
					matching = arg_matches?(remainingIds, args, i)
					remainingIds.delete_if { |id| matching.assoc(id).nil? }
					puts "remaining ids => #{remainingIds.inspect}" if debug_level >= DebugLevel::High
					puts "arg_matches => #{matching.inspect}" if debug_level >= DebugLevel::High
					break matching if remainingIds.length <= 1
				}
				if ! matching.nil?
					if matching.length == 1
						chosen = matching[0][0]
						puts "Resolved to id: #{methodIds[0]}" if debug_level >= DebugLevel::High
					else
						raise ArgumentError, "Ambiguous method call '#{method}'"
					end
				end
			end

			if chosen.nil? and method == classname
				puts "No matching constructor found, possibles:\n"
				id = find_pclassid(normalize_classname(klass.name))
				hash = findAllMethods(id)
				constructor_names = hash.keys.grep(/^#{classname}/)
				method_ids = hash.values_at(*constructor_names).flatten
				puts dumpCandidates(method_ids)
			end

			puts "setCurrentMethod(#{chosen})" if debug_level >= DebugLevel::High
			setCurrentMethod(chosen) if chosen
			return nil
		end

		def init()
			getClassList().each {
                                |c|
				if c == "Qt"
					# Don't change Qt to Qt::t, just leave as is
					@@cpp_names["Qt"] = c
				elsif c != "QInternal"
					init_class(c)
				end
                        }
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
                        matches = signal.sub(/\(.*/, '').include? signalName
			return [signal, i] if matches
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
			args = argStr.scan(/[^, ]+/)
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
