#!/usr/bin/env ruby

require 'Qt'
require 'dclock'

a = Qt::Application.new(ARGV)
clock = DigitalClock.new
clock.resize(170,80)
a.setMainWidget(clock)
clock.setCaption('QtRuby Example - Digital Clock')
clock.show
a.exec
