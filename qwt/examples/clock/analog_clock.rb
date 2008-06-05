require 'Qt4'
require 'qwt'

app = Qt::Application.new(ARGV)
clock = Qwt::AnalogClock.new
clock.scaleDraw().setPenWidth(3);
clock.lineWidth = 6
clock.frameShadow = Qwt::Dial::Sunken
clock.setTime
clock.show

# update the clock every second
timer = Qt::Timer.new(clock)
timer.connect(timer, SIGNAL(:timeout), clock, SLOT(:setCurrentTime))
timer.start(1000)
app.exec
