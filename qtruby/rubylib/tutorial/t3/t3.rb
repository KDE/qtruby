#!/usr/bin/env ruby -w
require 'Qt'

a = Qt::Application.new(ARGV)

box = Qt::VBox.new()
box.resize(200, 120)

quit = Qt::PushButton.new('Quit', box)
quit.setFont(Qt::Font.new('Times', 18, Qt::Font.Bold))

a.connect(quit, SIGNAL('clicked()'), SLOT('quit()'))

a.setMainWidget(box)
box.show

a.exec
exit
