#!/usr/bin/env ruby

require 'Korundum'



class MyBase < Qt::HBox
   slots "back()", "forward()"
   attr_accessor :back, :forward, :url
   def initialize *k
      super *k
      @url  = "http://www.kde.org/"
      @forward = KDE::PushButton.new(self) { setText "Hello World" }
      @back    = KDE::PushButton.new(self) { setText "Hello World" }
      @label   = Qt::Label.new(self)
      Qt::Object.connect( @forward, SIGNAL( "clicked()" ),
                          self,     SLOT(   "back()" ) )
      Qt::Object.connect( @back,    SIGNAL( "clicked()" ),
                          self,     SLOT(   "forward()" ) )
   end
   def back
      @label.setText "would go backwards"
   end
   def forward
      @label.setText "would go forwards"
   end
end

about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["four"], about)
a = KDE::Application.new()

browser = Qt::VBox.new

blah = MyBase.new browser
browser.show

require 'net/http'
require 'uri'

output = Net::HTTP.get URI.parse(blah.url)
p output
w = KDE::HTMLPart.new browser
w.begin
w.write output
w.end
w.show

a.setMainWidget(browser)
a.exec()


__END__

b0rked due to KDE::URL non existance...

url = KDE::URL.new "http://www.kde.org"
p url.site
w = KDE::HTMLPart.new
w.openURL url
w.view.resize 500, 400
w.show
=end

TESTCASE - 
w = KDE::HTMLPart  # notice the missing .new
w.begin
=> crashes badly

dum di dum

dcop = KDE::DCOPObject.new()
