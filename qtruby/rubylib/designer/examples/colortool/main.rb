require 'Qt'

require 'mainform.rb'
require 'mainform.ui.rb'

require 'colornameform.rb'
require 'colornameform.ui.rb'

require 'optionsform.rb'

require 'findform.rb'
require 'findform.ui.rb'

require 'qmake_image_collection.rb'
    
a = Qt::Application.new(ARGV)
w = MainForm.new
a.mainWidget = w
w.show
a.exec
