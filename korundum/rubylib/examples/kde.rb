#!/usr/bin/env ruby

require 'Korundum'

#
# in order to use KURL's as constants one must place this KApplication init 
# at the top of the file otherwise KInstance isn't init'ed before KURL usage
#
about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["four"], about)
a = KDE::Application.new()

# Qt::Internal::setDebug Qt::QtDebugChannel::QTDB_ALL
# Qt.debug_level = Qt::DebugLevel::High

class MyBase < Qt::VBox
   HOME = KDE::URL.new ENV["BASEDOCURL"]
   slots "go_back()", "go_forward()", "goto_url()", "go_home()", "debug()"
   attr_accessor :back, :forward, :url
   def initialize( *k )
      super *k
      buttons = Qt::HBox.new self
      @w = KDE::HTMLPart.new self
      @history = []
      @popped_history = []
      @url      = HOME
      @forward  = KDE::PushButton.new(buttons) { setText "Forward" }
      @back     = KDE::PushButton.new(buttons) { setText "Back" }
      @home     = KDE::PushButton.new(buttons) { setText "Home" }
      @debug    = KDE::PushButton.new(buttons) { setText "Debug" }
      @location = Qt::LineEdit.new buttons
      @label    = Qt::Label.new self
      Qt::Object.connect( @back,    SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_back()" ) )
      Qt::Object.connect( @forward, SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_forward()" ) )
      Qt::Object.connect( @home,    SIGNAL( "clicked()" ),
                          self,     SLOT(   "go_home()" ) )
      Qt::Object.connect( @debug,   SIGNAL( "clicked()" ),
                          self,     SLOT(   "debug()" ) )
      Qt::Object.connect( @location,SIGNAL( "returnPressed()" ),
                          self,     SLOT(   "goto_url()" ) )
      load_page
      update_ui_elements
      self.resize 800,600
   end
   def debug
      node = @w.document
      indent = 0
      until node.isNull
         puts "NODE NAME :: #{node.inspect}"
         if node.nodeType == 9 # DOM::Node::TEXT_NODE
            blah = Qt::Internal::cast_object_to(node, "KDE::DOM::Text")
            str = puts '"' + blah.data.string + '"'
         end
         if not node.firstChild.isNull
            node = node.firstChild
            indent += 1
         elsif not node.nextSibling.isNull
            node = node.nextSibling
         else
            while !node.isNull and node.nextSibling.isNull
               node = node.parentNode
               indent -= 1
            end
            if not node.isNull
               node = node.nextSibling
            end
         end 
      end
   end
   def load_page
      @w.setCaretMode true
      @w.openURL @url
      @w.show
      # @w.slotDebugDOMTree
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

Thread.new {
   puts "indexing"
}

browser = MyBase.new
browser.show
a.setMainWidget(browser)
a.exec()

__END__

TESTCASE - 
w = KDE::HTMLPart  # notice the missing .new
w.begin
=> crashes badly

./kde.rb:29:in `method_missing': Cannot handle 'const QIconSet&' as argument to QTabWidget::changeTab (ArgumentError)
        from ./kde.rb:29:in `initialize'
        from ./kde.rb:92:in `new'
        from ./kde.rb:92
for param nil given to param const QIconSet &
occurs frequently

dum di dum

can't get tabwidget working. umm... wonder what i'm messing up...

      tabwidget = KDE::TabWidget.new browser
      tabwidget.setTabPosition Qt::TabWidget::Top
      @w = KDE::HTMLPart.new tabwidget
      w2 = KDE::HTMLPart.new tabwidget
      tabwidget.changeTab @w, Qt::IconSet.new, "blah blah"
      tabwidget.showPage @w
      tabwidget.show
      @w.show

dcop = KDE::DCOPObject.new()
