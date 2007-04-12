=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

=begin
Copyright 2003 Jim Bublitz

Terms and Conditions

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Except as contained in this notice, the name of the copyright holder shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from the
copyright holder.
=end

require 'Korundum'

class MainWin < KDE::MainWindow
    def initialize(*args)
        super
	end
end

#-------------------- main ------------------------------------------------

appName = "template"

aboutData = KDE::AboutData.new("systry", "Systray test",\
    "0.1", "Systray test", KDE::AboutData::License_GPL,\
    "(c) 2002, Jim Bublitz")

aboutData.addAuthor('Jim Bublitz', 'Example for PyKDE', 'jbublitz@nwinternet.com')
aboutData.addAuthor('Richard Dale', 'For Korundum', 'Richard_Dale@tipitina.demon.co.uk')

# This MUST go here (before KApplication () is called)
KDE::CmdLineArgs.init(ARGV, aboutData)
app = KDE::Application.new()
mainWindow = MainWin.new(nil, "main window")
icons = KDE::IconLoader.new()

systray = KDE::SystemTray.new(mainWindow)
systray.setPixmap(icons.loadIcon("stop", 0))
systray.show()

#mainWindow.show()
app.exec()


