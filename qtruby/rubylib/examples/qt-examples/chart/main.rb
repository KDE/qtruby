require 'Qt'

require 'canvasview.rb'
require 'canvastext.rb'
require 'element.rb'
require 'chartform.rb'
require 'chartform_canvas.rb'
require 'chartform_files.rb'
require 'optionsform.rb'
require 'setdataform.rb'

app = Qt::Application.new( ARGV )

if app.ARGV.length > 0
    filename = app.ARGV[0]
	if filename.rindex( /.cht$/ ).nil?
	    filename = nil
    end
end

cf = ChartForm.new( filename )
app.mainWidget = cf
cf.show

app.exec

