#/usr/bin/ruby

require 'Qt4'
require 'qscintilla'

app = Qt::Application.new(ARGV)
w = Qsci::Scintilla.new
w.show

app.exec
