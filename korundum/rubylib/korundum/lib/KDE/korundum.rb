=begin
/***************************************************************************
                          Korundum.rb  -  KDE specific ruby runtime, dcop etc.
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003 by Richard Dale
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

	class DCOPMetaInfo
		attr_accessor :dcop_object, :k_dcop_signals, :k_dcop, :changed
		def initialize(aClass)
			DCOPMeta[aClass.name] = self
			@dcop_object = nil
			@k_dcop_signals = []
			@k_dcop = []
			@changed = false
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
		signalNames = []
		signals = DCOPMeta[classname].k_dcop_signals
				return [] if signals.nil?
				signals.each {
						|signal| signalNames << signal.sub(/\(.*/, '')
				}
		signalNames
	end

	def fullSignalName(instance, signalName)
		classname = instance.class.name if instance.class.is_a? Module
		signals = DCOPMeta[classname].k_dcop_signals
				signals.each {
						|signal|
						if signal.sub(/\(.*/, '') == signalName
							return signal
						end
				}
	end

	class RubyDCOPObject < KDE::DCOPObject
		def initialize(instance, functions)
			super(instance.class.name)
			@instance = instance
			@functions = functions
		end

		def process(fun, data, replyType, replyData)
			return KDE::dcop_process(@instance, fun.sub(/\(.*/, ''), Qt::getMocArguments(fun), data, replyType, replyData)
		end

		def interfaces()
			ifaces = super()
			return ifaces << @instance.class.name
		end

		def functions()
			functions = super()
			return functions << @functions
		end
	end

	# If a class contains a k_dcop slots list declaration, then create a DCOPObject
	# asccociated with it	
	def createDCOPObject(instance)
		meta = DCOPMeta[instance.class.name]
		return nil if meta.nil?

		if meta.dcop_object.nil? or meta.changed
			meta.dcop_object = RubyDCOPObject.new(instance, meta.k_dcop)
			meta.changed = false
		end

		meta.dcop_object
	end
end

class Module
	include KDE

	def k_dcop_signals(*signal_list)
		meta = DCOPMeta[self.name] || DCOPMetaInfo.new(self)
		meta.k_dcop_signals += signal_list
		meta.changed = true
	end

	def k_dcop(*slot_list)
		meta = DCOPMeta[self.name] || DCOPMetaInfo.new(self)
		meta.k_dcop += slot_list
		meta.changed = true
	end
end
