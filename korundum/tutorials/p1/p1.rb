require 'Qt4'

a = Qt::Application.new( ARGV )

hello = Qt::PushButton.new( "Hello world!", nil )
hello.resize( 100, 30 )
 
Qt::Object::connect( hello, SIGNAL('clicked()'), a, SLOT('quit()') )

Qt::Object::connect( hello, SIGNAL('clicked()'), a) { puts "hi there!" }
 
hello.show()
 
a.exec()