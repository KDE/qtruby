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

class MyBase < Qt::VBox
   HOME = KDE::URL.new "http://www.gentoo.org/"
   slots "go_back()", "go_forward()", "goto_url()", "go_home()"
   attr_accessor :back, :forward, :url
   def initialize *k
      super *k
      buttons = Qt::HBox.new self
      @w = KDE::HTMLPart.new self
      @history = []
      @popped_history = []
      @url     = KDE::URL.new "http://www.gentoo.org/"
      @forward = KDE::PushButton.new(buttons) { setText "Forward" }
      @back    = KDE::PushButton.new(buttons) { setText "Back" }
      @home    = KDE::PushButton.new(buttons) { setText "Home" }
      @label   = Qt::Label.new(buttons)
      @location = Qt::LineEdit.new(buttons)
      Qt::Object.connect( @back,    SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_back()" ) )
      Qt::Object.connect( @forward, SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_forward()" ) )
      Qt::Object.connect( @home,    SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_home()" ) )
      Qt::Object.connect( @location,SIGNAL( "returnPressed()" ),
                          self,     SLOT(   "goto_url()" ) )
      load_page
      update_ui_elements
   end
   def load_page
      @w.openURL @url
      self.resize 500,400
      @w.show
   end
   def update_ui_elements
      @forward.setDisabled @popped_history.empty?
      @back.setDisabled    @history.empty?
   end
   def go_back
      fail "ummm... already at the start, gui bug" if @history.empty?
      goto_url @history.pop, false
      @popped_history << @url
      update_loc
   end
   def go_forward
      fail "ummm... already at the end, gui bug" if @popped_history.empty?
      goto_url @popped_history.pop
      update_loc
   end
   def update_loc
      @location.setText @url.prettyURL
   end
   def go_home
      goto_url HOME
   end
   def goto_url url = nil, history_store = true
      @popped_history = []
      @url = KDE::URL.new (url.nil? ? @location.text : url)
      @label.setText "going somewhere - #{@url.prettyURL}"
      if history_store
         @history << @url
         update_ui_elements
      end
      load_page
      update_loc unless url.nil?
   end
end

browser = MyBase.new
browser.show
a.setMainWidget(browser)
a.exec()


__END__

TESTCASE - 
w = KDE::HTMLPart  # notice the missing .new
w.begin
=> crashes badly

dum di dum

dcop = KDE::DCOPObject.new()
