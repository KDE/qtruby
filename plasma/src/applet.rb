require 'plasma_applet'

module RubyAppletScript
  class Applet < Plasma::AppletScript
    def initialize(parent, args)
      super(parent)
    end

    def camelize(str)
      str.gsub(/(^|[_-])(.)/) { $2.upcase }
    end

    def init
      applet.resize(200, 200)
      puts "RubyAppletScript::Applet#init mainScript: #{mainScript}"
      program = Qt::FileInfo.new(mainScript)
      load Qt::File.encodeName(program.filePath).to_s
      moduleName = camelize(package.metadata.name)
      className = camelize(program.baseName)
      puts "RubyAppletScript::Applet#init instantiating: #{moduleName}::#{className}"
      klass = Object.const_get(moduleName.to_sym).const_get(className.to_sym)
      @applet_script = klass.new(self)
      @applet_script.init
      return true
    end

    def paintInterface(painter, option, contentsRect)
      @applet_script.paintInterface(painter, option, contentsRect)
    end

    def constraintsEvent(constraints)
      @applet_script.constraintsEvent(constraints)
    end

    def contextualActions
      @applet_script.contextualActions
    end

    def showConfigurationInterface
      @applet_script.showConfigurationInterface
    end
  end
end