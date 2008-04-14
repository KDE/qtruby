=begin
/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   Translated to Ruby by Richard Dale
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
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

class TimeEngine < Plasma::DataEngine

  def initialize(parent, args)
    super(parent)
    setMinimumUpdateInterval(333)

    # To have translated timezone names
    # (effectively a noop if the catalog is already present).
    KDE::Global.locale.insertCatalog("timezones4")
  end

  def sourceRequested(name)
    # puts "TimeEngine#sourceRequested #{name}"
    return updateSource(name)
  end

  def updateSource(tz)
    # puts "TimeEngine#updateTime"
    localName = I18N_NOOP("Local")
    if tz == localName
        setData(localName, I18N_NOOP("Time"), Qt::Variant.new(Qt::Time.currentTime))
        setData(localName, I18N_NOOP("Date"), Qt::Variant.new(Qt::Date.currentDate))
        # this is relatively cheap - KSTZ.local is cached
        timezone = KDE::SystemTimeZones.local.name
    else
        newTz = KDE::SystemTimeZones.zone(tz)
        unless newTz.valid?
            return false
        end

        dt = KDE::DateTime.currentDateTime(KDE::DateTime::Spec.new(newTz))
        setData(tz, I18N_NOOP("Time"), Qt::Variant.new(dt.time))
        setData(tz, I18N_NOOP("Date"), Qt::Variant.new(dt.date))
        timezone = tz
    end

    trTimezone = i18n(timezone)
    setData(tz, I18N_NOOP("Timezone"), Qt::Variant.new(trTimezone))
    tzParts = trTimezone.split("/")

    setData(tz, I18N_NOOP("Timezone Continent"), Qt::Variant.new(tzParts[0]))
    setData(tz, I18N_NOOP("Timezone City"), Qt::Variant.new(tzParts[1]))

    return true
  end
end
