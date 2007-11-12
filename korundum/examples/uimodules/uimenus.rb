=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

module UIMenus

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


class PageNotImpl
    def initialize(parent)
        page = parent.addPage()

        x = 10
        y = 10

        niLbl   = Qt::Label.new("Nothing is currently implemented for this widget", page)
        niLbl.setGeometry(x, y, 300, 20)
        niLbl.show()
	end
end

def UIMenus.menuKAccelGen(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMenus.menuKAccelMenu(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMenus.menuKAction(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKActionMenu(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKActionSeparator(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKContextMenuManager(parent)
#    pass
end

def UIMenus.menuKDCOPActionProxy(parent)
#    pass
end

def UIMenus.menuKHelpMenu(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKMenuBar(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKPanelApplet(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMenus.menuKPanelExtension(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMenus.menuKPanelMenu(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMenus.menuKPopupFrame(parent)
#    pass
end

def UIMenus.menuKPopupMenu(parent)
#    pass
end

def UIMenus.menuKPopupTitle(parent)
#    pass
end

def UIMenus.menuKStatusBar(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKStatusBarLabel(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKStdAction(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKToolBar(parent)
    parent.currentPageObj = PageLaunch.new(parent)
    system("ruby menudemo.rb")
end

def UIMenus.menuKWindowListMenu(parent)
#    pass
end


if $0 == __FILE__
    puts
    puts "Please run uisampler.rb"
    puts
end

end

