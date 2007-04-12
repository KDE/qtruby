require 'Qt'

    a = Qt::Application.new( ARGV )

    hello = Qt::PushButton.new( "Hello world!", nil )
    hello.resize( 100, 30 )
 
    Qt::Object::connect( hello, SIGNAL('clicked()'), a, SLOT('quit()') )
 
    a.setMainWidget( hello )
    hello.show()
 
    a.exec()