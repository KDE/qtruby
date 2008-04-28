=begin
/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick                          *
 *   siraj@kdemail.net                                                     *
 *                                                                         *
 *   Translated to Ruby by Richard Dale                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with self program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
=end

require 'plasma_applet'
require 'analog_clock_config.rb'

class AnalogClock < Plasma::Containment

  slots 'dataUpdated(QString,Plasma::DataEngine::Data)',
        'createConfigurationInterface(KConfigDialog*)',
        :moveSecondHand,
        :configAccepted

  def initialize(parent, args)
    super

    setHasConfigurationInterface(true)
    resize(125, 125)
    setRemainSquare(true)

    @theme = Plasma::Svg.new(self)
    @theme.imagePath = "widgets/clock"
    @theme.containsMultipleImages = false
    @theme.resize(size())

    @timezone = ""
    @showTimeString = false
    @showSecondHand = false
    @ui = Ui::AnalogClockConfig.new
    @lastTimeSeen = Qt::Time.new
  end

  def init
    cg = config()
    @showTimeString = cg.readEntry("showTimeString", Qt::Variant.new(false)).value
    @showSecondHand = cg.readEntry("showSecondHand", Qt::Variant.new(false)).value
    @fancyHands = cg.readEntry("fancyHands", Qt::Variant.new(false)).value
    @timezone = cg.readEntry("timezone", Qt::Variant.new("Local")).value

    connectToEngine()
    constraintsEvent(Plasma::AllConstraints)
  end

  def connectToEngine
    # Use 'dataEngine("ruby-time")' for the ruby version of the engine
    timeEngine = dataEngine("time")
    if @showSecondHand
        timeEngine.connectSource(@timezone, self, 500)
    else 
        timeEngine.connectSource(@timezone, self, 6000, Plasma::AlignToMinute)
    end
  end

  def constraintsEvent(constraints)
    if constraints.to_i & Plasma::FormFactorConstraint.to_i
      setBackgroundHints(NoBackground)
    end
  end

  def shape
    path = Qt::PainterPath.new
    path.addEllipse(boundingRect().adjusted(-2, -2, 2, 2))
    return path
  end

  def dataUpdated(source, data)
    @time = data["Time"].toTime()
    if @time.minute == @lastTimeSeen.minute &&
      @time.second == @lastTimeSeen.second
      # avoid unnecessary repaints
      return
    end

    if @secondHandUpdateTimer
        @secondHandUpdateTimer.stop
    end

    @lastTimeSeen = @time
    update()
  end

  def createConfigurationInterface(parent)
    # TODO: Make the size settable
    widget = Qt::Widget.new
    @ui.setupUi(widget)
    parent.buttons = KDE::Dialog::Ok | KDE::Dialog::Cancel | KDE::Dialog::Apply
    connect(parent, SIGNAL(:applyClicked), self, SLOT(:configAccepted))
    connect(parent, SIGNAL(:okClicked), self, SLOT(:configAccepted));
    parent.addPage(widget, parent.windowTitle, icon)

    @ui.timeZones.setSelected(@timezone, true)
    @ui.timeZones.enabled = @timezone != "Local"
    @ui.localTimeZone.checked = @timezone == "Local"
    @ui.showTimeStringCheckBox.checked = @showTimeString
    @ui.showSecondHandCheckBox.checked = @showSecondHand
  end

  def configAccepted()
    cg = config()
    @showTimeString = @ui.showTimeStringCheckBox.checked?
    @showSecondHand = @ui.showSecondHandCheckBox.checked?

    cg.writeEntry("showTimeString", Qt::Variant.new(@showTimeString))
    cg.writeEntry("showSecondHand", Qt::Variant.new(@showSecondHand))
    update()
    tzs = @ui.timeZones.selection

    if @ui.localTimeZone.checkState == Qt::Checked
      dataEngine("time").disconnectSource(@timezone, self)
      @timezone = "Local"
      cg.writeEntry("timezone", Qt::Variant.new(@timezone))
    elsif tzs.length > 0
      # TODO: support multiple timezones
      tz = tzs[0]
      if tz != @timezone
          dataEngine("time").disconnectSource(@timezone, self)
          @timezone = tz;
          cg.writeEntry("timezone", Qt::Variant.new(@timezone))
      end
    elsif @timezone != "Local"
      dataEngine("time").disconnectSource(@timezone, self)
      @timezone = "Local"
      cg.writeEntry("timezone", Qt::Variant.new(@timezone))
    end

    connectToEngine
    constraintsUpdated(Plasma::AllConstraints)
    emit configNeedsSaving
  end

  def moveSecondHand
    update
  end

  def drawHand(p, rotation, handName)
    p.save
    boundSize = boundingRect.size
    elementSize = @theme.elementSize(handName)

    p.translate(boundSize.width() / 2, boundSize.height() / 2)
    p.rotate(rotation)
    p.translate(-elementSize.width() / 2, -elementSize.width())
    @theme.paint(p, Qt::RectF.new(Qt::PointF.new(0.0, 0.0), Qt::SizeF.new(elementSize)), handName)
    p.restore
  end

  def paintInterface(p, option, rect)
    tempRect = Qt::RectF.new(0, 0, 0, 0)

    boundSize = geometry.size
    elementSize = Qt::Size.new

    p.renderHint = Qt::Painter::SmoothPixmapTransform

    minutes = 6.0 * @time.minute - 180
    hours = 30.0 * @time.hour - 180 + ((@time.minute / 59.0) * 30.0)

    @theme.paint(p, Qt::RectF.new(rect), "ClockFace")

    drawHand(p, hours, "HourHand")
    drawHand(p, minutes, "MinuteHand")

    # Make sure we paint the second hand on top of the others
    if @showSecondHand
      anglePerSec = 6.0
      seconds = anglePerSec * @time.second() - 180

      if @fancyHands
        if @secondHandUpdateTimer.nil?
          @secondHandUpdateTimer = Qt::Timer.new(self)
          connect(@secondHandUpdateTimer, SIGNAL(:timeout), self, SLOT(:moveSecondHand))
        end

        if !@secondHandUpdateTimer.active?
          @secondHandUpdateTimer.start(50)
          @animationStart = Qt::Time.currentTime.msec
        else
          runTime = 500
          m = 1.0 # Mass
          b = 1.0 # Drag coefficient
          k = 1.5 # Spring constant
          gamma = b / (2 * m) # Dampening constant
          omega0 = Math.sqrt(k / m)
          omega1 = Math.sqrt(omega0 * omega0 - gamma * gamma)
          elapsed = Qt::Time.currentTime().msec() - @animationStart
          t = (4 * Math::PI) * (elapsed / runTime)
          val = 1 + exp(-gamma * t) * -Math.cos(omega1 * t)

          if elapsed > runTime
            @secondHandUpdateTimer.stop
          else
            seconds += -anglePerSec + (anglePerSec * val)
          end
        end
      end

      drawHand(p, seconds, "SecondHand");
    end

    p.save
    @theme.resize(boundSize)
    elementSize = @theme.elementSize("HandCenterScrew")
    tempRect.setSize(Qt::SizeF.new(elementSize))
    p.translate(boundSize.width() / 2 - elementSize.width() / 2, boundSize.height() / 2 - elementSize.height() / 2)
    @theme.paint(p, tempRect, "HandCenterScrew")
    p.restore

    if @showTimeString
      if @showSecondHand
        # FIXME: temporary time output
        time = @time.toString
        fm = Qt::FontMetrics.new(Qt::Application.font)
        p.drawText((rect.width/2 - fm.width(time) / 2),
                  ((rect.height/2) - fm.xHeight*3), @time.toString)
      else
        time = @time.toString("hh:mm")
        fm = Qt::FontMetrics.new(Qt::Application.font)
        p.drawText((rect.width/2 - fm.width(time) / 2),
                  ((rect.height/2) - fm.xHeight*3), @time.toString("hh:mm"))
      end
    end

    @theme.paint(p, Qt::RectF.new(rect), "Glass")
  end

end
