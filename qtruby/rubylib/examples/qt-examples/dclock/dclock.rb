=begin
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**

** Translated to QtRuby by Richard Dale
=end

require 'Qt'

class DigitalClock < Qt::LCDNumber

	slots 'stopDate()', 'showTime()'

	# Constructs a DigitalClock widget
	def initialize
		super

		@showingColon = false
		setFrameStyle(Qt::Frame.Panel | Qt::Frame.Raised)
		setLineWidth(2)			# set frame line width
		showTime			# display the current time
		@normalTimer = startTimer(500)	# 1/2 second timer events
		@showDateTimer = -1		# not showingdate
	end

	# Handles timer events for the digital clock widget.
	# There are two different timers; one timer for updating the clock
	# and another one for switching back from date mode to time mode.
	def timerEvent (e)
		if (e.timerId == @showDateTimer)		# stop showing date
			stopDate
		else					# normal timer
			if (@showDateTimer == -1)	# not showing date
				showTime()
			end
		end
	end

	# Enters date mode when the left mouse button is pressed.
	def mousePressEvent (e)
		if (e.button == Qt::MouseEvent.LeftButton)	# left button pressed
			showDate
		end
	end

	def stopDate
		killTimer(@showDateTimer)
		@showDateTimer = -1
		showTime
	end

	def showTime
		@showingColon = !@showingColon	# toggle/blink colon
		s = Qt::Time.currentTime.toString[0..4]
		if (!@showingColon)
			s[2] = ' '
		end
		if (s[0] == '0')
			s[0] = ' '
		end
		display(s)			# set LCD number/text
	end

	def showDate
		if (@showDateTimer != -1)		# already showing date
			return
		end
		date = Qt::Date.currentDate
		s = sprintf('%2d %2d', date.month, date.day)
		display(s)				# sets the LCD number/text
		@showDateTimer = startTimer(2000)	# keep this state for 2 secs
	end

end
