#/usr/bin/ruby

require 'qscintilla'

app = Qt::Application.new(ARGV)
w = Qsci::Scintilla.new
w.show

app.exec
