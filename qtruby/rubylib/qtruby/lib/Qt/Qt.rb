module Qt
	
	module Internal
		Classes	= Hash.new
		CppName	= Hash.new
		IdClass	= Array.new
		
		def init_class(c)
			classname = c.sub(/^Q/, 'Qt::')			
			classId = idClass(c)
			insert_pclassid(classname, classId)
			IdClass[classId] = classname
			CppName[classname] = c
			if isQObject(classId)
				klass = create_qobject_class(classname)
			else
				klass = create_qt_class(classname)
			end
			
			if klass != nil
				Classes[classname] = klass
			end
		end
		
		def argmatch(methodIds, args, i)
			match = Hash.new
			argtype = getVALUEtype(args[i])
			for method in methodIds
				arg = args[i] 
				typename = getTypeNameOfArg(method, i)
				
				if argtype == 'i'
					if typename =~ /^(?:short|ushort|int|uint|long|ulong|signed|unsigned)$/
						match[method] = 0
					end
				elsif argtype == 'n'
					if typename =~ /^(?:float|double)$/
						match[method] = 0
					end
				elsif argtype == 's'
					if typename =~ /^(?:char\*|const char\*|(?:const )?(Q(C?)String)[*&]?)$/
#		$match{$method} = defined $2 ? 1 : ( defined $1 ? 2 : 0 );
						match[method] = 0
					end
				elsif argtype == 'a'
            # FIXME: shouldn't be hardcoded. Installed handlers should tell what perl type they expect.
#					if typename =~ /^(?:
#                                const\ QCOORD\*|
#                                (?:const\ )?
#                                (?:
#                                  QStringList[\*&]?|
#                                  QValueList<int>[\*&]?|
#                                  QRgb\*|
#                                  char\*\*
#                                )
#                              )$/x)
#						match[method] = 0
#					end
				elsif argtype == 'u'
					if typename =~ /^(?:float|double)$/
						match[method] = 0
					end
				elsif argtype == 'U'
	    			match[method] = 0
				else
					t = typename.sub(/^const\s+/, '')
					t.sub!(/[&*]$/, '')
					if classIsa(argtype, t)
						match[method] = 0
					end
				end
			end
			return match.sort {|a,b| a[1] <=> b[1]}
		end
		
		def find_class(classname)
			value = Classes[classname];
			return Classes[classname]
		end
		
		def try_initialize(instance, *args)
			initializer = instance.method(:initialize)
			return callcc {|continuation|
				@@current_initializer = continuation
				initializer.call(*args)
			}
		end
		
		def continue_new_instance(instance)
			@@current_initializer.call(instance)
		end
				
		def do_method_missing(package, method, klass, *args)
			classname = CppName[klass.name]
			if classname.nil? and klass != Object
				do_method_missing(package, method, klass.superclass, *args)
				return 
			end

			method_original = method.clone
			
			if method == "new"
				method = classname * 1
			end
						
			for arg in args
				if arg.nil? or isObject(arg)
					method << "#"
				elsif arg.kind_of? Array or arg.kind_of? Hash
					method << "?"
				else
					method << "$"
				end
			end
			
			methodIds = findMethod(classname, method)
			if methodIds.length > 1
				for i in 0..(args.length - 1)
					matching = argmatch(methodIds, args, i)
					# Match if there is either just a single result returned, or if there are
					# multiple matches and the first match is a better match than subequent ones
					if matching != nil and (matching.length == 1 or matching[0][1] != matching[1][1])
						methodIds[0] = matching[0][0]
#						print("Resolved Method #{classname}::#{method} => " + methodIds[0].to_s + "\n")
						break
					end
				end
			end

			setCurrentMethod(methodIds[0] || -1)
		end
		
		def init()
			classes = getClassList()
			for c in classes
				if c == "Qt"
					# Don't change Qt to Qt::t, just leave as is
					CppName["Qt"] = c
				elsif c != "QInternal"
					init_class(c)
				end
			end
		end
	end

	Meta = Hash.new

	class MetaInfo
		attr_accessor(:signals, :slots, :metaobject, :mocargs)
		def initialize(aClass)
			Meta[aClass.name] = self
			@metaobject = nil
			@signals = Array.new
			@slots = Array.new
		end
	end
		
	def hasMembers(aClass)
		classname = aClass.name if aClass.is_a? Module
		meta = Meta[classname]
		return meta != nil && (meta.signals.length > 0 or meta.slots.length > 0)
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
		signalNames = Array.new
		signals = Meta[classname].signals
		if signals != nil
			for signal in signals
				signalNames.push(signal.sub(/\(.*/, ''))
			end
		end
		return signalNames
	end
	
	def signalInfo(qobject, signalName)
		classname = qobject.class.name if qobject.class.is_a? Module
		signals = Meta[classname].signals
		i = 0
		result = Array.new
		for signal in signals
			if signal.sub(/\(.*/, '').include? signalName
				result.push(signal)
				result.push(i)
				return result
			end			
			i += 1
		end
	end
		
	def signalAt(qobject, index)
		classname = qobject.class.name if qobject.class.is_a? Module
		return Meta[classname].signals[index]
	end

	def slotAt(qobject, index)
		classname = qobject.class.name if qobject.class.is_a? Module
		return Meta[classname].slots[index]
	end
	
	def getMocArguments(member)
		argStr = member.sub(/.*\(/, '')
		argStr.sub!(/\)$/, '')
		args = argStr.scan(/[^, ]+/)
		mocargs = allocateMocArguments(args.length)
		i = 0
		for arg in args
			a = arg.sub(/^const\s+/, '')
	    	if a =~ /^(bool|int|double|char\*|QString)&?$/
				a = $1
			else
				a = 'ptr'
			end
			valid = setMocType(mocargs, i, arg, a)
			i += 1
		end
		result = Array.new
		result.push(args.length)
		result.push(mocargs)
		return result
	end
	
	def makeMetaData(data)
		if data.nil?
			return nil
		end
		
		tbl = Array.new()
		
		for entry in data
			params = Array.new()
			name = entry.sub(/\(.*/, '')
			argStr = entry.sub(/.*\(/, '')
			argStr.sub!(/\)$/, '')
			args = argStr.scan(/[^, ]+/)
			for arg in args
				name = ''
				param = make_QUParameter(name, arg, 0, 1)
				params.push(param)
			end
			method = make_QUMethod(name, params)
			tbl.push(make_QMetaData(entry, method))
		end
		
		return make_QMetaData_tbl(tbl)
	end
		
	def getMetaObject(qobject)
		meta = Meta[qobject.class.name]
		if meta.nil?
			return nil
		end

		if meta.metaobject.nil?
			slotTable = makeMetaData(meta.slots)
			signalTable = makeMetaData(meta.signals)
			meta.metaobject=(	make_metaObject(	qobject.class.name, 
													qobject.staticMetaObject(),
													slotTable, 
													meta.slots.length,
													signalTable, 
													meta.signals.length ) )
		end
		
		return meta.metaobject
	end
	
	def emit(signal)
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
	
	def SIGNAL(string)
		return "2" + string
	end
	
	def SLOT(string)
		return "1" + string
	end
	
	def emit(signal)
	end
end

class Module
	include Qt
	
	def signals(*signal_list)
		meta = Meta[self.name]
		if meta.nil?
			meta = MetaInfo.new(self)
		end
		meta.signals += signal_list
	end
	
	def slots(*slot_list)
		meta = Meta[self.name]
		if meta.nil?
			meta = MetaInfo.new(self)
		end
		meta.slots += slot_list
	end
end

