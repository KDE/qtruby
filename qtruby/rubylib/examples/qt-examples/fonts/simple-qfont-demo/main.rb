#!/usr/bin/env ruby

require 'Qt'
require 'viewer'


a = Qt::Application.new(ARGV)
n = 3 # get board size n

textViewer = Viewer.new
textViewer.setCaption('QtRuby Example - Simple QFont Demo')
a.setMainWidget(textViewer)
textViewer.show
a.exec()
