#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt'

class MyWidget < Qt::Widget

def initialize()
	super
    setMinimumSize(200, 120)
    setMaximumSize(200, 120)

    quit = Qt::PushButton.new('Quit', self, 'quit')
    quit.setGeometry(62, 40, 75, 30)
    quit.setFont(Qt::Font.new('Times', 18, Qt::Font.Bold))
    $qApp.connect(quit, SIGNAL('clicked()'), SLOT('quit()'))
end

end

a = Qt::Application.new(ARGV)

w = MyWidget.new
w.setGeometry(100, 100, 200, 120)
a.setMainWidget(w)
w.show
a.exec
