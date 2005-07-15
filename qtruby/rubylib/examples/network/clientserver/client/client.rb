require 'Qt'

class Client < Qt::VBox

    def initialize( host, port )
		super()
		# GUI layout
		@infoText = Qt::TextView.new( self )
		hb = Qt::HBox.new( self )
		@inputText = Qt::LineEdit.new( hb )
		send = Qt::PushButton.new( tr("Send") , hb )
		close = Qt::PushButton.new( tr("Close connection") , self )
		quit = Qt::PushButton.new( tr("Quit") , self )
	
		connect( send, SIGNAL('clicked()'), SLOT('sendToServer()') )
		connect( close, SIGNAL('clicked()'), SLOT('closeConnection()') )
		connect( quit, SIGNAL('clicked()'), $qApp, SLOT('quit()') )
	
		# create the socket and connect various of its signals
		@socket = Qt::Socket.new( self )
		connect( @socket, SIGNAL('connected()'),
			SLOT('socketConnected()') )
		connect( @socket, SIGNAL('connectionClosed()'),
			SLOT('socketConnectionClosed()') )
		connect( @socket, SIGNAL('readyRead()'),
			SLOT('socketReadyRead()') )
		connect( @socket, SIGNAL('error(int)'),
			SLOT('socketError(int)') )
	
		# connect to the server
		@infoText.append( tr("Trying to connect to the server\n") )
		@socket.connectToHost( host, port )
    end
	
	slots	'closeConnection()', 'sendToServer()',
			'socketReadyRead()', 'socketConnected()',
			'socketConnectionClosed()', 'socketClosed()',
			'socketError(int)'

    def closeConnection()
		@socket.close()
		if @socket.state() == Qt::Socket::Closing
			# We have a delayed close.
			connect( @socket, SIGNAL('delayedCloseFinished()'),
				SLOT('socketClosed()') )
		else
			# The socket is closed.
			socketClosed()
		end
    end

    def sendToServer()
		# write to the server
		os = Qt::TextStream.new(@socket)
		os << @inputText.text() << "\n"
		@inputText.setText( "" )
		os.dispose()
    end

    def socketReadyRead()
		# read from the server
		while @socket.canReadLine() do
	    	@infoText.append( @socket.readLine() )
		end
    end

    def socketConnected()
		@infoText.append( tr("Connected to server\n") )
    end

    def socketConnectionClosed()
		@infoText.append( tr("Connection closed by the server\n") )
    end

    def socketClosed()
		@infoText.append( tr("Connection closed\n") )
    end

    def socketError( e )
		@infoText.append( tr("Error number %d occurred\n" % e) )
    end
end

app = Qt::Application.new( ARGV )
client = Client.new( ARGV.length < 1 ? "localhost" : ARGV[0], 4242 )
app.mainWidget = client
client.show
app.exec
