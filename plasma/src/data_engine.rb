require 'plasma_applet'

module Ruby_Script_Engine
  class DataEngine < Plasma::DataEngineScript
    def sourceRequestEvent(name)
      @data_engine.sourceRequestEvent(name)
    end

    def updateSourceEvent(source)
      @data_engine.updateSourceEvent(source)
    end
  end
end