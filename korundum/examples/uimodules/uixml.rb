=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

module UIXML

class PageLaunch
    def initialize(parent)
        page = parent.addPage()

        x = 10
        y = 10

        launchLbl   = Qt::Label.new("Launching application ... please wait\n\nClose launched application to continue", page)
        launchLbl.setGeometry(x, y, 300, 80)
        launchLbl.show()

        page.show()

        $kapp.processEvents()
	end
end

def UIXML.xmlKActionCollection(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby xmlmenudemo.rb")
end

def UIXML.xmlKEditToolbar(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby xmlmenudemo.rb")
end

def UIXML.xmlKEditToolbarWidget(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby xmlmenudemo.rb")
end

def UIXML.xmlKXMLGUIBuilder(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby xmlmenudemo.rb")
end

def UIXML.xmlKXMLGUIClient(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby xmlmenudemo.rb")
end

def UIXML.xmlKXMLGUIFactory(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby xmlmenudemo.rb")
end

end
