#!/usr/bin/ruby -w
require 'Qt'
require "gamebrd.rb"


Qt::Application.setColorSpec( Qt::Application.CustomColor )
a = Qt::Application.new(ARGV)

gb = GameBoard.new
gb.setGeometry( 100, 100, 500, 355 )
a.setMainWidget(gb)
gb.show
a.exec
