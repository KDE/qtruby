#!/usr/bin/env ruby

require 'Korundum'

# Qt.debug_level = Qt::DebugLevel::High

class MyBase < Qt::HBox
   slots "back()", "forward()"
   attr_accessor :back, :forward, :url
   def initialize *k
      super *k
      @url  = "http://www.gnome.org/"
      @forward = KDE::PushButton.new(self) { setText "Back" }
      @back    = KDE::PushButton.new(self) { setText "Forward" }
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

url = KDE::URL.new "http://www.gentoo.org"
w = KDE::HTMLPart.new browser
w.openURL url
w.view.resize 500, 400
w.show

a.setMainWidget(browser)
a.exec()


__END__

TESTCASE - 
w = KDE::HTMLPart  # notice the missing .new
w.begin
=> crashes badly

dum di dum

dcop = KDE::DCOPObject.new()
