require 'plasma_applet'

module RubyAppletScript
  class DataEngine < Plasma::DataEngineScript
    def initialize(parent, args)
      super(parent)
    end

    def camelize(str)
      str.gsub(/(^|[_-])(.)/) { $2.upcase }
    end

    def init
      puts "RubyAppletScript::DataEngine#init mainScript: #{mainScript}"
      program = Qt::FileInfo.new(mainScript)
      load Qt::File.encodeName(program.filePath).to_s
      moduleName = camelize(package.metadata.name)
      className = camelize(program.baseName)
      puts "RubyAppletScript::DataEngine#init instantiating: #{moduleName}::#{className}"
      klass = Object.const_get(moduleName.to_sym).const_get(className.to_sym)
      @data_engine_script = klass.new(self)
      @data_engine_script.init
      return true
    end

    def sourceRequestEvent(name)
      @data_engine.sourceRequestEvent(name)
    end

    def updateSourceEvent(source)
      @data_engine.updateSourceEvent(source)
    end
  end
end