#!/usr/bin/env ruby

require 'Korundum'

#
# in order to use KURL's as constants one must place this KApplication init 
# at the top of the file otherwise KInstance isn't init'ed before KURL usage
#
about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["four"], about)
a = KDE::Application.new()

# Qt.debug_level = Qt::DebugLevel::High

class MyBase < Qt::HBox
   HOME = KDE::URL.new "http://www.gentoo.org/"
   slots "go_back()", "go_forward()", "goto_url()", "go_home()"
   attr_accessor :back, :forward, :url
   def initialize *k
      super *k
      @history = []
      @popped_history = []
      @url     = KDE::URL.new "http://www.gentoo.org/"
      @forward = KDE::PushButton.new(self) { setText "Forward" }
      @back    = KDE::PushButton.new(self) { setText "Back" }
      @home    = KDE::PushButton.new(self) { setText "Home" }
      @label   = Qt::Label.new(self)
      @location = Qt::LineEdit.new(self)
      Qt::Object.connect( @back,    SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_back()" ) )
      Qt::Object.connect( @forward, SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_forward()" ) )
      Qt::Object.connect( @home,    SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_home()" ) )
      Qt::Object.connect( @location,SIGNAL( "returnPressed()" ),
                          self,     SLOT(   "goto_url()" ) )
      update_history
   end
   def update_history
      @forward.setDisabled @popped_history.empty?
      @back.setDisabled    @history.empty?
   end
   def go_back
      fail "ummm... already at the start, gui bug" if @history.empty?
      @url = @history.pop; update_history
      @popped_history << @url
      @label.setText " < going somewhere - #{@url.prettyURL}"
      update_loc
   end
   def go_forward
      fail "ummm... already at the end, gui bug" if @popped_history.empty?
      @url = @popped_history.pop
      @history << @url; update_history
      @label.setText " > going somewhere - #{@url.prettyURL}"
      update_loc
   end
   def update_loc
      @location.setText @url.prettyURL
   end
   def go_home
      goto_url HOME
   end
   def goto_url url = nil
      @popped_history = []
      @url = KDE::URL.new (url.nil? ? @location.text : url)
      @label.setText "going somewhere - #{@url.prettyURL}"
      @history << @url; update_history
      update_loc unless url.nil?
   end
end

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
