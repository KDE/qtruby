=begin
/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <riccardo@kde.org>           *
 *   Copyright (C) 2007 by Sebastian Kuegler <sebas@kde.org>               *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
=end

require 'plasma_applet'
require 'digital_clock_config.rb'
require 'calendar.rb'

class DigitalClock < Plasma::Applet
  PlainClock = 0
  FancyClock = 1

  slots 'dataUpdated(QString,Plasma::DataEngine::Data)',
        'showCalendar(QGraphicsSceneMouseEvent *)',
        :showConfigurationInterface,
        :configAccepted,
        :updateColors

  def initialize(parent, args)
    super
    @clockStyle = PlainClock
    @plainClockFont = KDE::GlobalSettings.generalFont
    @useCustomColor = false
    @plainClockColor = Qt::Color.new(Qt::white)
    @plainClockFontBold = false
    @plainClockFontItalic = false
    @showDate = false
    @showYear = false
    @showDay = false
    @showSeconds = false
    @showTimezone = false
    @lastTimeSeen = Qt::Time.new
    @ui = Ui::DigitalClockConfig.new
    @calendarUi = Ui::Calendar.new

    setHasConfigurationInterface(true)
    setContentSize(70, 22)
  end

  def init
    cg = config
    @timezone = cg.readEntry("timezone", Qt::Variant.new("Local")).value
    @timeZones = cg.readEntry("timeZones", [])

    @showTimezone = cg.readEntry("showTimezone", Qt::Variant.new((@timezone != "Local"))).value

    @showDate = cg.readEntry("showDate", Qt::Variant.new(false)).value
    @showYear = cg.readEntry("showYear", Qt::Variant.new(false)).value

    @showDay = cg.readEntry("showDay", Qt::Variant.new(true)).value

    @showSeconds = cg.readEntry("showSeconds", Qt::Variant.new(false)).value
    @plainClockFont = cg.readEntry("plainClockFont", Qt::Variant.fromValue(@plainClockFont)).value
    @useCustomColor = cg.readEntry("useCustomColor", Qt::Variant.new(false)).value
    if @useCustomColor
        @plainClockColor = cg.readEntry("plainClockColor", Qt::Variant.fromValue(@plainClockColor)).value
    else
        @plainClockColor = KDE::ColorScheme.new(Qt::Palette::Active, KDE::ColorScheme::View, Plasma::Theme.self.colors).foreground.color
    end
    @plainClockFontBold = cg.readEntry("plainClockFontBold", Qt::Variant.new(true)).value
    @plainClockFontItalic = cg.readEntry("plainClockFontItalic", Qt::Variant.new(false)).value
    @plainClockFont.setBold(@plainClockFontBold)
    @plainClockFont.setItalic(@plainClockFontItalic)

    metrics = Qt::FontMetricsF.new(KDE::GlobalSettings.smallestReadableFont)
    timeString = KDE::Global.locale.formatTime(Qt::Time.new(23, 59), @showSeconds)
    setMinimumContentSize(metrics.size(Qt::TextSingleLine, timeString))

    @toolTipIcon = KDE::Icon.new("chronometer").pixmap(KDE::IconSize(KDE::IconLoader::Desktop))

    # Use 'dataEngine("ruby-time")' for the ruby version of the engine
    dataEngine("time").connectSource(@timezone, self, updateInterval, intervalAlignment)
    connect(Plasma::Theme.self, SIGNAL(:changed), self, SLOT(:updateColors))
  end

  def expandingDirections
    if formFactor == Plasma::Horizontal
      return Qt::Vertical
    else
      return Qt::Horizontal
    end
  end

  def dataUpdated(source, data)
    @time = data["Time"].toTime
    @date = data["Date"].toDate
    @prettyTimezone = data["Timezone City"].toString

    timeString = KDE::Global.locale.formatTime(@time, @showSeconds)

    tipData = Plasma::ToolTipData.new
    tipData.mainText = @time.toString(timeString)
    tipData.subText = @date.toString
    tipData.image = @toolTipIcon

    setToolTip(tipData)

    # avoid unnecessary repaints
    if @showSeconds || @time.minute != @lastTimeSeen.minute
        @lastTimeSeen = @time
        update
    end
  end

  def mousePressEvent(event)
    if event.buttons == Qt::LeftButton && contentRect.contains(event.pos)
        showCalendar(event)
    else
        event.ignore
    end
  end

  def showCalendar(event)
    if @calendar == 0
        @calendar = Plasma::Dialog.new
        # @calendar.setStyleSheet("{ border : 0px }") # FIXME: crashes
        @layout = Qt::VBoxLayout.new
        @layout.spacing = 0
        @layout.margin = 0

        @calendarUi.setupUi(@calendar)
        @calendar.layout = @layout
        @calendar.windowFlags = Qt::Popup
        @calendar.adjustSize
    end

    if @calendar.visible?
        @calendar.hide
    else
        @calendarUi.kdatepicker.date = Qt::Date.currentDate
        @calendar.move(popupPosition(@calendar.sizeHint))
        @calendar.show
    end
  end

  def showConfigurationInterface
    if @dialog.nil?
        @dialog = KDE::Dialog.new

        @dialog.caption = i18n("Configure Clock")

        widget = Qt::Widget.new
        @ui.setupUi(widget)
        @dialog.mainWidget = widget
        @dialog.buttons = KDE::Dialog::Ok | KDE::Dialog::Cancel | KDE::Dialog::Apply

        connect( @dialog, SIGNAL(:applyClicked), self, SLOT(:configAccepted) )
        connect( @dialog, SIGNAL(:okClicked), self, SLOT(:configAccepted) )
    end
    @ui.showDate.checked = @showDate
    @ui.showYear.checked = @showYear
    @ui.showDay.checked = @showDay
    @ui.secondsCheckbox.checked = @showSeconds
    @ui.showTimezone.checked = @showTimezone
    @ui.plainClockFontBold.checked = @plainClockFontBold
    @ui.plainClockFontItalic.checked = @plainClockFontItalic
    @ui.plainClockFont.currentFont = @plainClockFont
    @ui.useCustomColor.checked = @useCustomColor
    @ui.plainClockColor.color = @plainClockColor
    @ui.timeZones.enabled = @timezone != "Local"
    @ui.localTimeZone.checked = @timezone == "Local"
    @timeZones.each do |str|
        @ui.timeZones.setSelected(str, true)
    end

    @dialog.show
  end

  def configAccepted
    cg = config
    #We need this to happen before we disconnect/reconnect sources to ensure
    #that the update interval is set properly.
    @showSeconds = @ui.secondsCheckbox.checkState == Qt::Checked
    cg.writeEntry("showSeconds", Qt::Variant.new(@showSeconds))
    #QGraphicsItem::update
    @timeZones = @ui.timeZones.selection
    cg.writeEntry("timeZones", Qt::Variant.new(@timeZones))
    if @ui.localTimeZone.checkState == Qt::Checked
        dataEngine("time").disconnectSource(@timezone, self)
        @timezone = "Local";
        dataEngine("time").connectSource(@timezone, self, updateInterval, intervalAlignment)
        cg.writeEntry("timezone", @timezone)
    elsif @timeZones.length > 0
        tz = @timeZones.at(0)
        if tz != @timezone
            dataEngine("time").disconnectSource(@timezone, self)
            # We have changed the timezone, show that in the clock, but only if this
            # setting hasn't been changed.
            @ui.showTimezone.checkState = Qt::Checked
            @timezone = tz
            dataEngine("time").connectSource(@timezone, self, updateInterval, intervalAlignment)
        end
        cg.writeEntry("timezone", Qt::Variant.new(@timezone))
    elsif @timezone != "Local"
        dataEngine("time").disconnectSource(@timezone, self)
        @timezone = "Local"
        dataEngine("time").connectSource(@timezone, self, updateInterval, intervalAlignment)
        cg.writeEntry("timezone", @timezone)
    else
        puts "User didn't use local timezone but also didn't select any other."
    end

    @showDate = @ui.showDate.checkState == Qt::Checked
    cg.writeEntry("showDate", Qt::Variant.new(@showDate))
    @showYear = @ui.showYear.checkState == Qt::Checked
    cg.writeEntry("showYear", Qt::Variant.new(@showYear))
    @showDay = @ui.showDay.checkState == Qt::Checked
    cg.writeEntry("showDay", Qt::Variant.new(@showDay))
    @showSeconds = @ui.secondsCheckbox.checkState == Qt::Checked
    cg.writeEntry("showSeconds", Qt::Variant.new(@showSeconds))

    if @showTimezone != (@ui.showTimezone.checkState == Qt::Checked)
        @showTimezone = @ui.showTimezone.checkState == Qt::Checked
        cg.writeEntry("showTimezone", Qt::Variant.new(@showTimezone))
        puts "Saving show timezone: %s" % @showTimezone;
    end

    @plainClockFont = @ui.plainClockFont.currentFont
    @useCustomColor = @ui.useCustomColor.checkState == Qt::Checked;
    if @useCustomColor
        @plainClockColor = @ui.plainClockColor.color
    else
        @plainClockColor = KDE::ColorScheme.new(Qt::Palette::Active, KDE::ColorScheme::View, Plasma::Theme.self.colors).foreground.color
    end
    @plainClockFontBold = @ui.plainClockFontBold.checkState == Qt::Checked;
    @plainClockFontItalic = @ui.plainClockFontItalic.checkState == Qt::Checked;

    @plainClockFont.setBold(@plainClockFontBold)
    @plainClockFont.setItalic(@plainClockFontItalic)

    cg.writeEntry("plainClock", Qt::Variant.new(@clockStyle == PlainClock))
    cg.writeEntry("plainClockFont", Qt::Variant.fromValue(@plainClockFont))
    cg.writeEntry("useCustomColor", Qt::Variant.new(@useCustomColor))
    cg.writeEntry("plainClockColor", Qt::Variant.fromValue(@plainClockColor))
    cg.writeEntry("plainClockFontBold", Qt::Variant.new(@plainClockFontBold))
    cg.writeEntry("plainClockFontItalic", Qt::Variant.new(@plainClockFontItalic))

    update
    emit configNeedsSaving
  end


  def paintInterface(p, option, contentsRect)
    if @time.valid? && @date.valid?
        p.font = KDE::GlobalSettings.smallestReadableFont

        p.pen = Qt::Pen.new(@plainClockColor)
        p.renderHint = Qt::Painter::SmoothPixmapTransform
        p.renderHint = Qt::Painter::Antialiasing

        timeRect = Qt::Rect.new

        # Paint the date, conditionally, and let us know afterwards how much
        # space is left for painting the time on top of it.
        if @showDate || @showTimezone
            dateString = ""
            if @showDate
                day = @date.toString("d")
                month = @date.toString("MMM")

                if @showYear
                    year = @date.toString("yyyy")
                    dateString = i18nc("@label Short date: " +
                                       "%s day in the month, %2 short month name, %3 year" % [day, month, year],
                                       "%s %s %s" % [day, month, year])
                else
                    dateString = i18nc("@label Short date: " +
                                       "%s day in the month, %s short month name" % [day, month],
                                       "%s %s" % [day, month])
                end

                if @showDay
                    weekday = Qt::Date.shortDayName(@date.dayOfWeek)
                    dateString = i18nc("@label Day of the week with date: " +
                                       "%s short day name, %s short date" % [weekday, dateString],
                                       "%s, %s" % [weekday, dateString])
                end

                if @showTimezone
                    timezone = @prettyTimezone
                    timezone.gsub!("_", " ")
                    dateString = i18nc("@label Date with timezone: " +
                                       "%s day of the week with date, %s timezone" % [dateString, timezone],
                                       "%s %s" % [dateString, timezone])
                end
            elsif @showTimezone
                dateString = @prettyTimezone
                dateString.gsub!("_", " ")
            end

            # Check sizes
            dateRect = preparePainter(p, contentsRect, KDE::GlobalSettings.smallestReadableFont, dateString)
            subtitleHeight = dateRect.height
            p.drawText(Qt::RectF.new(0,
                                contentsRect.bottom-subtitleHeight,
                                contentsRect.right,
                                contentsRect.bottom) ,
                        dateString,
                        Qt::TextOption.new(Qt::AlignHCenter)
                    )

            # Now find out how much space is left for painting the time
            timeRect = Qt::Rect.new(   contentsRect.left,
                                contentsRect.top,
                                contentsRect.width,
                                (contentsRect.height-subtitleHeight+4))
        else
            timeRect = contentsRect
        end

        timeString = KDE::Global.locale.formatTime(@time, @showSeconds)

        @plainClockFont.bold = @plainClockFontBold
        @plainClockFont.italic = @plainClockFontItalic

        # Choose a relatively big font size to start with
        @plainClockFont.pointSizeF = [timeRect.height, KDE::GlobalSettings.smallestReadableFont.pointSize].max
        preparePainter(p, timeRect, @plainClockFont, timeString)

        p.drawText(Qt::RectF.new(timeRect),
                    timeString,
                    Qt::TextOption.new(Qt::AlignCenter)
                )
    end
  end

  def preparePainter(p, rect, font, text)
    tmpRect = Qt::Rect.new
    tmpFont = font

    # Starting with the given font, decrease its size until it'll fit in the
    # given rect allowing wrapping where possible
    begin
        p.font = tmpFont
        tmpFont.pointSize = [KDE::GlobalSettings.smallestReadableFont.pointSize, tmpFont.pointSize - 1].max
        tmpRect = p.boundingRect(rect, Qt::TextWordWrap, text)
    end while tmpFont.pointSize > KDE::GlobalSettings.smallestReadableFont.pointSize && (tmpRect.width > rect.width ||
            tmpRect.height > rect.height)

    return tmpRect
  end

  def updateInterval
    return @showSeconds ? 1000 : 60000
  end

  def intervalAlignment
    return @showSeconds ? Plasma::NoAlignment : Plasma::AlignToMinute;
  end

  def updateColors
    if !@useCustomColor
        @plainClockColor = KDE::ColorScheme(Qt::Palette::Active, KDE::ColorScheme::View, Plasma::Theme.self.colors).foreground.color
        update
    end
  end
end

