#!/usr/bin/env ruby

require 'Qt'
require 'hello'

a = Qt::Application.new(ARGV)
s = ''

s = ARGV[1..ARGV.size-1].join(' ')

if (s.empty?)
	s = 'Hello, World'
end

h = Hello.new(s)
h.setCaption('QtRuby says hello')
h.connect(h, SIGNAL('clicked()'), a, SLOT('quit()'))
h.setFont(Qt::Font.new('times', 32, Qt::Font.Bold))	# default font
h.setBackgroundColor(Qt::white)				# default bg color
a.setMainWidget(h)
h.show

a.exec
