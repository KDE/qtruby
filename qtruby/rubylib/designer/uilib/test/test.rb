require 'Qt'
require 'qui'

a = Qt::Application.new(ARGV)
if ARGV.length == 0
	puts "Usage: test.rb <image dir> <ui file>"
	exit
end

if ARGV.length == 2
	QUI::WidgetFactory.loadImages( ARGV[ 0 ] )
    w = QUI::WidgetFactory.create( ARGV[ 1 ] )
    if w.nil?
		puts "Failed to create top level widget"
		exit
	end
    w.show()
    a.connect( a, SIGNAL('lastWindowClosed()'), a, SLOT('quit()') )
all = a.allWidgets
all.each { |w| p w }
    a.exec()
end
