#!/usr/bin/env ruby

require 'Qt'
require 'aclock'

app = Qt::Application.new(ARGV)
clock = AnalogClock.new
clock.show
app.exec
