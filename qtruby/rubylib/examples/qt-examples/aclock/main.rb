#!/usr/bin/env ruby

require 'Qt'
require 'aclock'

a = Qt::Application.new(ARGV)
clock = AnalogClock.new
ARGV.each {|arg|
	clock.setAutoMask(true) if arg == '-transparent'
}
clock.resize(100, 100)
a.setMainWidget(clock)
clock.setCaption('QtRuby example - Analog Clock')
clock.show
a.exec
