#!/usr/bin/env ruby

require 'Qt'
require 'tooltip'

a = Qt::Application.new(ARGV)
n = 3 # get board size n

mw = TellMe.new
mw.setCaption('QtRuby Example - Dynamic Tool Tips')
a.setMainWidget(mw)
mw.show
a.exec
