#!/usr/bin/env ruby

require 'Korundum'

class MyBase < Qt::Object
   slots "blah()"
   def initialize mybutton
      super
      @knop  = mybutton
      @count = 0
   end
   def blah
      @knop.setText "#{@count+=1}"
   end
end

about = KDE::AboutData.new("one", "two", "three")
KDE::CmdLineArgs.init(1, ["four"], about)
a = KDE::Application.new()
hello = KDE::PushButton.new(nil) { setText "Hello World" }
blah = MyBase.new hello
Qt::Object.connect( hello, SIGNAL( "clicked()" ),
                    blah,  SLOT( "blah()" ) )

=begin
b0rked due to KDE::URL non existance...
--
url = KDE::URL.new "http://www.kde.org"
p url.site
w = KDE::HTMLPart.new
w.openURL url
w.view.resize 500, 400
w.show
=end

w = KDE::HTMLPart.new
# TESTCASE - w = KDE::HTMLPart; then w.begin; crashes badly
w.begin
w.write "blah <b>blah</b> blah "
w.end
w.show()

a.setMainWidget(w)
dcop = KDE::DCOPObject.new()
w.show()
a.exec()
