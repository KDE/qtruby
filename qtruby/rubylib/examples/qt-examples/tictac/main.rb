#!/usr/bin/env ruby

require 'Qt'
require 'tictac'

a = Qt::Application.new(ARGV)
n = 3 # get board size n

ttt = TicTacToe.new(n)
a.setMainWidget(ttt)
ttt.setCaption('QtRuby Example - TicTac')
ttt.show()
a.exec()
