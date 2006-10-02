=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

=begin
This is a minimal PyKDE app template - it constructs an application
and a main window, but does nothing else.
=end

=begin
Copyright 2003 Jim Bublitz

Terms and Conditions

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KDE::IND, EXPRESS OR
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


=begin
Most Korundum applications will need a main window - the is the top
level widget (the parent for all other widgets). KDE::MainWindow has
more functionality than shown here (see more complex templates).
It has the ability to create the other major parts of the user
interface - the main view, menus, toolbars, etc.

Usually you provide a subclass of KDE::MainWindow, construct menus
and toolbars in the subclass' initialize method, and provide
slots for menu/toolbar actions in separate methods.
=end
class MainWin < KDE::MainWindow
    def initialize(*args)
        super
	end
end


#-------------------- main ------------------------------------------------

# instantiate KDE::Application - no other Qt::Object
# or Qt::Widget based classes can be instantiated
# until there is a KDE::Application instance
appName = "template"
about = KDE::AboutData.new(appName, "A minimal application", "0.1")
KDE::CmdLineArgs.init(ARGV, about)
app = KDE::Application.new()

# instantiate the subclass of KMainWindow
mainWindow = MainWin.new(nil, "main window")

# create the display
mainWindow.show

# run KDE::Application's event loop until the
# program exits
app.exec


