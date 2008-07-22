require 'plasma_applet'

module Tiger
  class Main < PlasmaScripting::Applet

    slots 'void dataUpdated(QString, Plasma::DataEngine::Data)'

    def initialize(parent, args = nil)
      super
    end

    def init
      @svg = Plasma::Svg.new(self)
      @svg.imagePath = 'widgets/tiger'
      timeEngine = dataEngine("time")
      timeEngine.connectSource("Local", self, 6000)
    end

    def dataUpdated(name, data)
      puts ("In DataUpdated name: %s data: %s", [name, data["Time"].toTime.toString]);
    end

    def paintInterface(painter, option, contentsRect)
      @svg.resize(size())
      @svg.paint(painter, 0, 0)
    end
  end
end
