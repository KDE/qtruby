=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

=begin
This program tests/demos some of the KSharedPtr related classes and
methods (KMimeType, KService, etc). It generally tests the *::List
methods for these classes (eg KService::List) since that also tests
the *::Ptr mapped type code (eg KService::Ptr) at the same time.

This version is suitable for KDE >= 3.0.0 (some methods not available
in earlier versions)
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
include KDE

class MainWin < MainWindow
    def initialize(*k)
		super

        tabctl = KDE::TabCtl.new(self)
        setGeometry(0, 0, 600, 400)
        tabctl.setGeometry(10, 10, 550, 380)

        tabctl.addTab(KMimeTypeTab.new(tabctl), "KMimeType")
        tabctl.addTab(KServiceTab.new(tabctl), "KService")
        tabctl.addTab(KSycocaEntryTab.new(tabctl), "KSycocaEntry")
        tabctl.addTab(KServiceTypeTab.new(tabctl), "KServiceType")
        tabctl.addTab(OfferListTab.new(tabctl), "OfferList")

        tabctl.show()
	end
end


class OfferListTab < Qt::Widget
    def initialize(parent, name = "")
        super(parent, name)

        setGeometry(0, 0, 500, 370)
        lvLbl = Qt::Label.new("Offers - text/html", self)
        lvLbl.setGeometry(10, 10, 150, 20)

        lv = Qt::ListView.new(self)
        lv.setSorting(-1)
        lv.addColumn("type", 75)
        lv.addColumn("name", 100)
        lv.addColumn("exec", 200)
        lv.addColumn("library", 100)
        lv.setGeometry(10, 30, 500, 300)
        lv.setAllColumnsShowFocus(1)

        # insert list items in reverse order

        pref = KDE::ServiceTypeProfile.preferredService("Application", "image/jpeg")
        Qt::ListViewItem.new(lv, pref.type(), pref.name(), pref.exec(), pref.library())
        Qt::ListViewItem.new(lv, "Preferred", "--------", "", "")
        Qt::ListViewItem.new(lv, "", "", "", "")

        trader = KDE::Trader.self()
        slist = trader.query("image/jpeg", "Type == 'Application'")
#        print "KTrader returned:" + slist
        slist.each do |s|
            lvi = Qt::ListViewItem.new(lv, s.type(), s.name(), s.exec(), s.library())
		end

        lv.show()
	end
end

class KServiceTypeTab < Qt::Widget
    def initialize(parent, name = "")
        super(parent, name)

        setGeometry(0, 0, 500, 370)
        lvLbl = Qt::Label.new("All Service Types", self)
        lvLbl.setGeometry(10, 10, 250, 20)

        lv = Qt::ListView.new(self)
        lv.addColumn("name", 150)
        lv.addColumn("desktopEntryPath", 300)
        lv.setGeometry(10, 30, 500, 300)
        lv.setAllColumnsShowFocus(1)

        slist = KDE::ServiceType.allServiceTypes()

        slist.each do |s|
            lvi = Qt::ListViewItem.new(lv, s.name(), s.desktopEntryPath())
		end

        lv.show()
	end
end

class KSycocaEntryTab < Qt::Widget
    def initialize(parent, name = "")
        super(parent, name)

        grp = KDE::ServiceGroup.baseGroup("screensavers")
		if grp.nil?
			return
		end
        setGeometry(0, 0, 500, 370)
        lvLbl = Qt::Label.new("Entries - 'screensavers': " + grp.name(), self)
        lvLbl.setGeometry(10, 10, 250, 20)

        lv = Qt::ListView.new(self)
        lv.addColumn("name", 150)
        lv.addColumn("entryPath", 300)
        lv.setGeometry(10, 30, 500, 300)
        lv.setAllColumnsShowFocus(1)

        slist = grp.entries(0, 0)

        slist.each do |s|
            lvi = Qt::ListViewItem.new(lv, s.name(), s.entryPath())
		end

        lv.show()
	end
end

class KServiceTab < Qt::Widget
    def initialize(parent, name = "")
        super(parent, name)

        setGeometry(0, 0, 500, 370)
        lvLbl = Qt::Label.new("All Services", self)
        lvLbl.setGeometry(10, 10, 150, 20)

        lv = Qt::ListView.new(self)
        lv.addColumn("type", 75)
        lv.addColumn("name", 100)
        lv.addColumn("exec", 200)
        lv.addColumn("library", 100)
        lv.setGeometry(10, 30, 500, 300)
        lv.setAllColumnsShowFocus(1)

        slist = KDE::Service.allServices()
        slist.each do |s|
            lvi = Qt::ListViewItem.new(lv, s.type(), s.name(), s.exec(), s.library())
		end

        lv.show()
	end
end

#        svc = KService.serviceByDesktopName ("kcookiejar")
#        print svc
#        print svc.type_ ()
#        print svc.name ().latin1 ()
#        print svc.exec_ ().latin1 ()
#        print svc.library ()


class KMimeTypeTab < Qt::Widget
    def initialize(parent, name = "")
        super(parent, name)

        setGeometry(0, 0, 500, 370)
        lbLbl = Qt::Label.new("All Mimetypes", self)
        lbLbl.setGeometry(10, 10, 150, 20)
        lb = KDE::ListBox.new(self)
        lb.setGeometry(10, 30, 200, 300)
        mlist = KDE::MimeType.allMimeTypes()
        lblist = []
		mlist.each do |mt|
            lblist << mt.name()
		end

        lblist.sort()
        lb.insertStringList(lblist)

        lb.show()

        x = 250
        y = 10

        mt = KDE::MimeType.mimeType("text/plain")
        mtlbl = Qt::Label.new("KMimeType.mimeType('text/plain')", self)
        mtlbl.setGeometry(x, y, 250, 20)
        mtnamelbl = Qt::Label.new("name", self)
        mtnamelbl.setGeometry(x + 15, y + 20, 100, 20)
        mtname = Qt::Label.new(mt.name(), self)
        mtname.setGeometry(x + 120, y + 20, 100, 20)
        mtdesklbl = Qt::Label.new("desktopEntryPath", self)
        mtdesklbl.setGeometry(x + 15, y + 40, 100, 20)
        mtdesk = Qt::Label.new(mt.desktopEntryPath(), self)
        mtdesk.setGeometry(x + 120, y + 40, 150, 20)

        y = y + 80

        fp = KDE::MimeType.findByPath("mimetype.rb")
        fplbl = Qt::Label.new("KDE::MimeType.findByPath('mimetype.rb')", self)
        fplbl.setGeometry(x, y, 250, 20)
        fpnamelbl = Qt::Label.new("name", self)
        fpnamelbl.setGeometry(x + 15, y + 20, 100, 20)
        fpname = Qt::Label.new(fp.name(), self)
        fpname.setGeometry(x + 120, y + 20, 100, 20)
        fpdesklbl = Qt::Label.new("desktopEntryPath", self)
        fpdesklbl.setGeometry(x + 15, y + 40, 100, 20)
        fpdesk = Qt::Label.new(fp.desktopEntryPath(), self)
        fpdesk.setGeometry(x + 120, y + 40, 150, 20)

        y = y + 80

        fu = KDE::MimeType.findByURL(KDE::URL.new("file://mimetype.rb"))
        fulbl = Qt::Label.new("KDE::MimeType.findByURL('file://mimetype.rb')", self)
        fulbl.setGeometry(x, y, 250, 20)
        funamelbl = Qt::Label.new("name", self)
        funamelbl.setGeometry(x + 15, y + 20, 100, 20)
        funame = Qt::Label.new(fu.name(), self)
        funame.setGeometry(x + 120, y + 20, 100, 20)
        fudesklbl = Qt::Label.new("desktopEntryPath", self)
        fudesklbl.setGeometry(x + 15, y + 40, 100, 20)
        fudesk = Qt::Label.new(fu.desktopEntryPath(), self)
        fudesk.setGeometry(x + 120, y + 40, 150, 20)

        y = y + 80

		acc = Qt::Integer.new(0) # Create a mutable integer value to pass as an 'int*' arg
        fc = KDE::MimeType.findByFileContent("mimetype.rb", acc)
        fclbl = Qt::Label.new("KDE::MimeType.findByFileContent('mimetype.rb')", self)
        fclbl.setGeometry(x, y, 250, 20)
        fcnamelbl = Qt::Label.new("name", self)
        fcnamelbl.setGeometry(x + 15, y + 20, 100, 20)
        fcname = Qt::Label.new(fc.name(), self)
        fcname.setGeometry(x + 120, y + 20, 100, 20)
        fcdesklbl = Qt::Label.new("desktopEntryPath", self)
        fcdesklbl.setGeometry(x + 15, y + 40, 100, 20)
        fcdesk = Qt::Label.new(fc.desktopEntryPath(), self)
        fcdesk.setGeometry(x + 120, y + 40, 100, 20)
        fcacclbl = Qt::Label.new("accuracy", self)
        fcacclbl.setGeometry(x + 15, y + 60, 100, 20)
        fcacc = Qt::Label.new(acc.to_s, self)
        fcacc.setGeometry(x + 120, y + 60, 150, 20)
	end
end


#-------------------- main ------------------------------------------------

description = "Test/demo KSharedPtr related methods/classes"
version     = "1.0"
aboutData   = KDE::AboutData.new("mimetype", "MimeType",
    version, description, KDE::AboutData.License_GPL,
    "(C) 2003 Jim Bublitz")

KDE::CmdLineArgs.init(ARGV, aboutData)

KDE::CmdLineArgs.addCmdLineOptions([["+files", "File to open", ""]])

app = KDE::Application.new()
mainWindow = MainWin.new(nil, "main window")
mainWindow.show()
app.exec()
