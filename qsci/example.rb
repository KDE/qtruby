#/usr/bin/ruby

require 'Qt'
require 'QScintilla'

app = Qt::Application.new(ARGV)
w = Qsci::Scintilla.new
w.show

app.exec
