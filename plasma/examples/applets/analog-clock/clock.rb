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
require 'calendar'

module PlasmaRubyAnalogClock

class Clock < Plasma::Containment

  slots :moveSecondHand, :configAccepted, 
        'showCalendar(QGraphicsSceneMouseEvent *)',
        'dataUpdated(QString, Plasma::DataEngine::Data)'

  def initialize(parent, args)
    super
    KDE::Global.locale.insertCatalog("libplasmaclock")

    setHasConfigurationInterface(true)
    resize(125, 125)
    setAspectRatioMode(Plasma::Square)

    @theme = Plasma::Svg.new(self)
    @theme.imagePath = "widgets/clock"
    @theme.containsMultipleImages = false
    @theme.resize(size())

    @timezone = ""
    @showTimeString = false
    @showSecondHand = false
    @ui = Ui::AnalogClockConfig.new
    @calendarUi = Ui::Calendar.new

    @lastTimeSeen = Qt::Time.new
    @calendar = 0
    @time = Qt::Time.new
  end

  def init
    cg = config()
    @showTimeString = cg.readEntry("showTimeString", false)
    @showSecondHand = cg.readEntry("showSecondHand", false)
    @fancyHands = cg.readEntry("fancyHands", false)
    @timezone = cg.readEntry("timezone", "Local")

    connectToEngine()
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

    cg.writeEntry("showTimeString", @showTimeString)
    cg.writeEntry("showSecondHand", @showSecondHand)
    update()
    tzs = @ui.timeZones.selection

    if @ui.localTimeZone.checkState == Qt::Checked
      dataEngine("time").disconnectSource(@timezone, self)
      @timezone = "Local"
      cg.writeEntry("timezone", @timezone)
    elsif tzs.length > 0
      # TODO: support multiple timezones
      tz = tzs[0]
      if tz != @timezone
          dataEngine("time").disconnectSource(@timezone, self)
          @timezone = tz;
          cg.writeEntry("timezone", @timezone)
      end
    elsif @timezone != "Local"
      dataEngine("time").disconnectSource(@timezone, self)
      @timezone = "Local"
      cg.writeEntry("timezone", @timezone)
    end

    connectToEngine
    constraintsEvent(Plasma::AllConstraints)
    emit configNeedsSaving
  end

  def moveSecondHand
    update
  end

  def drawHand(p, rotation, handName)
    p.save
    boundSize = boundingRect.size
    elementRect = @theme.elementRect(handName)

    p.translate(boundSize.width() / 2, boundSize.height() / 2)
    p.rotate(rotation)
    p.translate(-elementRect.width / 2, -(@theme.elementRect("clockFace").center.y - elementRect.top))
    @theme.paint(p, Qt::RectF.new(Qt::PointF.new(0.0, 0.0), elementRect.size), handName)
    p.restore
  end

  def paintInterface(p, option, rect)
    tempRect = Qt::RectF.new(0, 0, 0, 0)
    boundSize = geometry.size
    p.renderHint = Qt::Painter::SmoothPixmapTransform

    minutes = 6.0 * @time.minute - 180
    hours = 30.0 * @time.hour - 180 + ((@time.minute / 59.0) * 30.0)

    @theme.paint(p, Qt::RectF.new(rect), "ClockFace")

    if @showTimeString
      fm = Qt::FontMetrics.new(Qt::Application.font)
      margin = 4
      if @showSecondHand
        # FIXME: temporary time output
        time = @time.toString
      else
        time = @time.toString("hh:mm")
      end
      textRect = Qt::Rect.new((rect.width/2 - fm.width(time) / 2),((rect.height / 2) - fm.xHeight * 4),
                  fm.width(time), fm.xHeight())

      p.pen = Qt::NoPen
      background = Plasma::Theme.defaultTheme.color(Plasma::Theme::BackgroundColor)
      background.setAlphaF(0.5)
      p.brush = Qt::Brush.new(background)

      p.setRenderHint(Qt::Painter::Antialiasing, true)
      p.drawPath(Plasma.roundedRectangle(Qt::RectF.new(textRect.adjusted(-margin, -margin, margin, margin)), margin))
      p.setRenderHint(Qt::Painter::Antialiasing, false)

      p.pen = Plasma::Theme::defaultTheme.color(Plasma::Theme::TextColor)
        
      p.drawText(textRect.bottomLeft, time)
    end

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
    end

    if @theme.hasElement("HourHandShadow")
        p.translate(1,3)

        drawHand(p, hours, "HourHandShadow")
        drawHand(p, minutes, "MinuteHandShadow")

        if @showSecondHand
            drawHand(p, seconds, "SecondHandShadow")
        end

        p.translate(-1,-3);
    end

    drawHand(p, hours, "HourHand")
    drawHand(p, minutes, "MinuteHand")

    if @showSecondHand
        drawHand(p, seconds, "SecondHand")
    end

    p.save
    @theme.resize(boundSize)
    elementSize = Qt::SizeF.new(@theme.elementSize("HandCenterScrew"))

    tempRect.size = elementSize
    p.translate(boundSize.width / 2.0 - elementSize.width / 2.0, boundSize.height / 2.0 - elementSize.height / 2.0)
    @theme.paint(p, tempRect, "HandCenterScrew")
    p.restore

    @theme.paint(p, Qt::RectF.new(rect), "Glass")
  end

  def mousePressEvent(event)
    if event.buttons == Qt::LeftButton
      showCalendar(event)
    else
      event.ignore
    end
  end

  def showCalendar(event)
    if @calendar == 0
      @calendar = Plasma::Dialog.new
      @calendarUi.setupUi(@calendar)
      @calendar.setWindowFlags(Qt::Popup)
      @calendar.adjustSize
    end

    if @calendar.visible?
      @calendar.hide
    else
      data = dataEngine("time").query(@timezone)
      @calendarUi.kdatepicker.date = data["Date"].toDate
      @calendar.move(popupPosition(@calendar.sizeHint))
      @calendar.show
    end
  end

end

end