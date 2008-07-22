require 'Qt4'

a = Qt::Application.new( ARGV )

hello = Qt::PushButton.new( "Hello world!", nil )
hello.resize( 100, 30 )
 
Qt::Object::connect( hello, SIGNAL('clicked()'), a, SLOT('quit()') )

Qt::Object::connect( hello, SIGNAL('clicked()'), a) { puts "hi there!" }
 
hello.show()

list = [1,2,3].map{|i| Qt::Variant.new i}
v = Qt::Variant.new list
p v
p v.value

a.exec()