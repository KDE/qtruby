#!/usr/bin/env ruby -w
require 'Qt';

a = Qt::Application.new(ARGV)

quit = Qt::PushButton.new("Quit", nil)
quit.resize(75, 30)
quit.setFont(Qt::Font.new("Times", 18, Qt::Font.Bold))

a.connect(quit, SIGNAL('clicked()'), SLOT('quit()'))

a.setMainWidget(quit)
quit.show
a.exec
exit
