#!/usr/bin/env ruby

require 'Qt'
require 'tooltip'

a = Qt::Application.new(ARGV)

mw = TellMe.new
mw.setCaption('QtRuby Example - Dynamic Tool Tips')
a.setMainWidget(mw)
mw.show
a.exec
