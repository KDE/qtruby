require 'Qt'

=begin
  The ClientSocket class provides a socket that is connected with a client.
  For every client that connects to the server, the server creates a new
  instance of this class.
=end
class ClientSocket < Qt::Socket
	def initialize(sock, parent=nil, name=nil)
		super( parent, name )
		@line = 0
		connect( self, SIGNAL('readyRead()'),
			SLOT('readClient()') )
		connect( self, SIGNAL('connectionClosed()'),
			SLOT('deleteLater()') )
		setSocket( sock )
    end

	signals 'logText(const QString&)'

	slots 'readClient()'

    def readClient()
		ts = Qt::TextStream.new( self )
		while canReadLine() do
			str = ts.readLine()
			emit logText( tr("Read: '%s'\n" % str) )
	
			ts << @line << ": " << str
			# 'endl' needs to be called like this in ruby
			endl(ts)
			emit logText( tr("Wrote: '%d: %s'\n" % [@line, str]) )
	
			@line += 1
		end
		ts.dispose()
    end
end


=begin
  The SimpleServer class handles new connections to the server. For every
  client that connects, it creates a new ClientSocket -- that instance is now
  responsible for the communication with that client.
=end
class SimpleServer < Qt::ServerSocket
	def initialize( parent=nil )
		super( 4242, 1, parent )
		if !ok()
			qWarning("Failed to bind to port 4242")
			exit(1)
		end
    end

    def newConnection( socket )
		s = ClientSocket.new( socket, self )
		emit newConnect( s )
    end

	# The type of the argument is 'QSocket*', not
	# 'ClientSocket*' as only types in the Smoke
	# library can be used for types in Signals
	signals 'newConnect(QSocket*)'
end


=begin
  The ServerInfo class provides a small GUI for the server. It also creates the
  SimpleServer and as a result the server.
=end
class ServerInfo < Qt::VBox
	def initialize()
		super
		@server = SimpleServer.new( self )
	
		itext = tr(
			"This is a small server example.\n" +
			"Connect with the client now."
			)
		lb = Qt::Label.new( itext, self )
		lb.setAlignment( AlignHCenter )
		@infoText = Qt::TextView.new( self )
		quit = Qt::PushButton.new( tr("Quit") , self )
	
		# See the comment above about why the 'ClientSocket*'
		# type cannot be used
		connect( @server, SIGNAL('newConnect(QSocket*)'),
			SLOT('newConnect(QSocket*)') )
		connect( quit, SIGNAL('clicked()'), $qApp,
			SLOT('quit()') )
    end

	slots 'newConnect(QSocket*)', 'connectionClosed()'

    def newConnect( s )
		@infoText.append( tr("New connection\n") )
		connect( s, SIGNAL('logText(const QString&)'),
			@infoText, SLOT('append(const QString&)') )
		connect( s, SIGNAL('connectionClosed()'),
			SLOT('connectionClosed()') )
    end

    def connectionClosed()
		@infoText.append( tr("Client closed connection\n") )
    end
end


app = Qt::Application.new( ARGV )
info = ServerInfo.new
app.mainWidget = info
info.show
app.exec


