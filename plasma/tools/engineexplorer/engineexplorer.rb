=begin
/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   Translated to Ruby by Richard Dale
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
=end

require 'plasma_applet'
require 'ui_engineexplorer.rb'

class EngineExplorer < KDE::Dialog

  slots 'dataUpdated(QString, Plasma::DataEngine::Data)',
        'showEngine(QString)',
        'addSource(QString)',
        'removeSource(QString)',
        'requestSource()'

  def initialize(parent = nil)
    super(parent)
    @engine = nil 
    @sourceCount = 0
    @engineName = ""

    setButtons(0)
    setWindowTitle(i18n("Plasma Engine Explorer"))
    mainWidget = Qt::Widget.new(super)
    setMainWidget(mainWidget)
    @ui = Ui::EngineExplorer.new
    @ui.setupUi(mainWidget)

    @engineManager = Plasma::DataEngineManager.new
    @dataModel = Qt::StandardItemModel.new(self)
    pix = KDE::Icon.new("plasma")
    size = KDE::IconSize(KDE::IconLoader::Dialog)
    @ui.m_title.setPixmap(pix.pixmap(size, size))
    connect(@ui.m_engines, SIGNAL('activated(QString)'), self, SLOT('showEngine(QString)'))
    connect(@ui.m_sourceRequesterButton, SIGNAL('clicked(bool)'), self, SLOT(:requestSource))
    @ui.m_data.model = @dataModel

    listEngines
    @ui.m_engines.setFocus

    addAction(KDE::StandardAction.quit($qApp, SLOT(:quit), self))
  end

  def engine=(engine)
    # find the engine in the combo box
    index = @ui.m_engines.findText(engine)
    if index != -1
        puts "Engine found!";
        @ui.m_engines.currentIndex = index
        showEngine(engine)
    end
  end

  def interval=(interval)
    @ui.m_updateInterval.value = interval
  end

  def dataUpdated(source, data)
    items = @dataModel.findItems(source, 0)

    if items.length < 1
        return
    end

    parent = items.first

    while parent.hasChildren do
        parent.removeRow(0)
    end

    showData(parent, data)
  end

  def listEngines
    @ui.m_engines.clear
    @ui.m_engines.addItem("")
    engines = @engineManager.knownEngines
    engines.sort!
    @ui.m_engines.addItems(engines)
  end

  def showEngine(name)
    @ui.m_sourceRequester.enabled = false
    @ui.m_sourceRequesterButton.enabled = false
    @dataModel.clear
    @dataModel.columnCount = 3
    headers = []
    headers << i18n("DataSource") << i18n("Key") << i18n("Value")
    @dataModel.setHorizontalHeaderLabels(headers)
    @engine = 0
    @sourceCount = 0

    if @engineName.empty?
        @engineManager.unload(@engineName)
    end

    @engineName = name
    if @engineName.empty?
        updateTitle
        return
    end

    @engine = @engineManager.load(@engineName)
    if @engine.nil?
        @engineName.clear
        updateTitle
        return
    end

    sources = @engine.sources

    # puts "showing engine #{@engine.objectName}"
    # puts "we have #{sources.count} data sources"
    sources.each do |source|
        # puts "adding #{source}"
        addSource(source)
    end

    @ui.m_sourceRequesterButton.enabled = true
    @ui.m_updateInterval.enabled = true
    @ui.m_sourceRequester.enabled = true
    @ui.m_sourceRequester.setFocus
    connect(@engine, SIGNAL('newSource(QString)'), self, SLOT('addSource(QString)'))
    connect(@engine, SIGNAL('sourceRemoved(QString)'), self, SLOT('removeSource(QString)'))
    updateTitle
  end

  def addSource(source)
    parent = Qt::StandardItem.new(source)
    @dataModel.appendRow(parent)

    # puts "getting data for source #{source}"
    data = @engine.query(source)
    showData(parent, data)
    @engine.connectSource(source, self)

    @sourceCount += 1
    updateTitle
  end

  def removeSource(source)
    items = @dataModel.findItems(source, 0)

    if items.length < 1
        return
    end

    items.each do |item|
        @dataModel.removeRow(item.row)
    end

    @sourceCount =- 1
    updateTitle
  end

  def requestSource
    if @engine.nil?
        return
    end

    source = @ui.m_sourceRequester.text

    if source.empty?
        return
    end

    @engine.connectSource(source, self, @ui.m_updateInterval.value)
  end

  def convertToString(value)
    if value.canConvert(Qt::Variant::String)
        return value.toString
    end

    case value.type
    when Qt::Variant::Point
      point = value.toPoint
      return "(%d, %d)" % [point.x, point.y]
    else
      return "<unknown>"
    end
  end

  def showData(parent, data)
    rowCount = 0
    data.each_pair do |key, value|
#    parent.insertRows(0, data.count)
#    parent.setColumnCount(3)
      parent.setChild(rowCount, 1, Qt::StandardItem.new(key))
      if value.canConvert(Qt::Variant::List)
        value.toList.each do |var|
          parent.setChild(rowCount, 2, Qt::StandardItem.new(convertToString(var)))
          rowCount += 1
        end
      else
        parent.setChild(rowCount, 2, Qt::StandardItem.new(convertToString(value)))
        rowCount += 1
      end
    end
  end

  def updateTitle
    if @engine.nil?
        @ui.m_title.pixmap = KDE::Icon.new("plasma").pixmap(KDE::IconSize(KDE::IconLoader::Dialog))
        @ui.m_title.text = i18n("Plasma DataEngine Explorer")
        return
    end

    @ui.m_title.text = i18nc("The name of the engine followed by the number of data sources",
                           "%s - %s data sources" % [@engine.objectName, @sourceCount])
    if @engine.icon.nil?
        @ui.m_title.pixmap = KDE::Icon.new("plasma").pixmap(KDE::IconSize(KDE::IconLoader::Dialog))
    else
        # @ui.m_title.pixmap = KDE::Icon.new("alarmclock").pixmap(KDE::IconSize(KDE::IconLoader::Dialog))
        @ui.m_title.pixmap = KDE::Icon.new(@engine.icon).pixmap(KDE::IconSize(KDE::IconLoader::Dialog))
    end
  end
end

description= I18N_NOOP("Explore the data published by Plasma DataEngines")
version = "0.0"
aboutData = KDE::AboutData.new("plasmaengineexplorer", nil, KDE::ki18n("Plasma Engine Explorer"),
                         version, KDE::ki18n(description), KDE::AboutData::License_GPL,
                         KDE::ki18n("(c) 2006, The KDE Team"))
aboutData.addAuthor(KDE::ki18n("Aaron J. Seigo"),
                    KDE::ki18n( "Author and maintainer" ),
                    "aseigo@kde.org")

KDE::CmdLineArgs.init(ARGV, aboutData)

options = KDE::CmdLineOptions.new
options.add("height <pixels>", KDE::ki18n("The desired height in pixels"))
options.add("width <pixels>", KDE::ki18n("The desired width in pixels"))
options.add("x <pixels>", KDE::ki18n("The desired x position in pixels"))
options.add("y <pixels>", KDE::ki18n("The desired y position in pixels"))
options.add("engine <data engine>", KDE::ki18n("The data engine to use"))
options.add("interval <ms>", KDE::ki18n("Update Interval in milliseconds.  Default: 50ms"), "50")
KDE::CmdLineArgs.addCmdLineOptions(options)

args = KDE::CmdLineArgs.parsedArgs

app = KDE::Application.new
w = EngineExplorer.new

# get pos if available
x = args.getOption("height").to_i
y = args.getOption("width").to_i
if x && y
  w.resize(x,y)
end

# get size
x = args.getOption("x").to_i
y = args.getOption("y").to_i
if x && y
  w.move(x,y)
end

# set interval
interval = args.getOption("interval").to_i
if interval
  w.interval = interval
end

# set engine
engine = args.getOption("engine")
if !engine.empty?
  w.engine = engine
end

args.clear

w.show
app.exec

