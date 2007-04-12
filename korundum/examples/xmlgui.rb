#!/usr/bin/env ruby

require 'Korundum'
require 'tempfile'

about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["RubberDoc"], about)
app = KDE::Application.new()

class Receiver < Qt::Object
   slots "pressed_up()", "close()", "quit()"
   def initialize main, *k
      super(*k)
      @main = main
   end
   def quit
      @main.close
   end
end

RAction = Struct.new :xmlgui_name, :string, :accel, :something
class RAction
   def create receiver, slot, action_collection
      p self.string
      KDE::Action.new self.string, self.accel, receiver, slot, action_collection, self.xmlgui_name
   end
end

# { Quit,          KStdAccel::Quit, "file_quit", I18N_NOOP("&Quit"), 0, "exit" },
std_actions = { :quit => RAction.new( "file_quit", ("&Quit"), KDE::Shortcut.new(), "exit" ) }

begin
   m = KDE::MainWindow.new
   @r = Receiver.new m
   mActionCollection = m.actionCollection
   action = std_actions[:quit].create @r, SLOT("quit()"), mActionCollection
   m.createGUI Dir.pwd + "/xmlgui.rc"
   app.setMainWidget(m)
   m.show
   app.exec()
end
