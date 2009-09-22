=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

module UIWidgets

class Page1 < Qt::Object
	slots 'restrict(int)'
	
    def initialize(parent)
		super
        page = parent.addPage()

        x = 10
        y = 10

        editLbl   = Qt::Label.new("KTextEdit", page)
        editLbl.setGeometry(x, y, 50, 20)
        editLbl.show()

        textList = ["Now is the winter of our discontent\n",
                    "made glorious summer by this sun of York;\n",
                    "and all the clouds that lour'd upon our house\n",
                    "in the deep bosom of the ocean buried.\n"]

        parent.edit = KDE::TextEdit.new(page)
        parent.edit.setGeometry(x, y + 20, 300, 100)
		textList.each do |line|
			parent.edit.insertPlainText(line)
		end
        parent.edit.show()

        y = y + 125
        searchBtn  = Qt::PushButton.new("Search", page)
        replaceBtn = Qt::PushButton.new("Replace", page)
        gotoBtn    = Qt::PushButton.new("GoTo Line", page)

        searchBtn.setGeometry(x, y, 60, 22)
        replaceBtn.setGeometry(x + 90, y, 60, 22)
        gotoBtn.setGeometry(x + 180, y, 60, 22)

#        page.connect(searchBtn, SIGNAL("clicked()"), parent.edit, SLOT('search()'))
#        page.connect(replaceBtn, SIGNAL("clicked()"), parent.edit, SLOT('replace()'))
#        page.connect(gotoBtn, SIGNAL("clicked()"), parent.edit, SLOT('doGotoLine()'))

        searchBtn.show()
        replaceBtn.show()
        gotoBtn.show()

        y = y + 35

        lineeditLbl   = Qt::Label.new("KLineEdit", page)
        lineeditLbl.setGeometry(x, y, 70, 20)
        lineeditLbl.show()

        lineedit = KDE::LineEdit.new(page)
        lineedit.setGeometry(x, y + 20, 100, 20)
        lineedit.show()

        intLbl   = Qt::Label.new("KIntNumInput", page)
        intLbl.setGeometry(x + 195, y + 35, 95, 20)
        intLbl.show()

        intNum = KDE::IntNumInput.new(5, page)
        intNum.setGeometry(x + 195, y + 55, 175, 50)
#        intNum.setSuffix(" GB")
#        intNum.setPrefix("hdc    ")
        intNum.setLabel("Capacity")
        intNum.setRange(0, 10)
#        intNum.setRange(0, 10, 1, true)
        intNum.show()

        y = y + 50

        dblLbl   = Qt::Label.new("KDoubleNumInput", page)
        dblLbl.setGeometry(x + 195, y + 80, 115, 20)
        dblLbl.show()

        dblNum = KDE::DoubleNumInput.new(page)
        dblNum.setValue(2.5)
        dblNum.setGeometry(x + 195, y + 100, 175, 50)
        dblNum.setLabel("Variable")
#        dblNum.setRange(0.0, 10.0, 0.5, true)
        dblNum.setRange(0.0, 10.0)
        dblNum.show()

        restricteditLbl   = Qt::Label.new("KRestrictedLine", page)
        restricteditLbl.setGeometry(x, y, 95, 20)
        restricteditLbl.show()

        @restrictlineedit = KDE::RestrictedLine.new(page)
        @restrictlineedit.setGeometry(x, y + 20, 100, 20)
        @restrictlineedit.show()

        buttons = ["Numbers Only", "Upper Case Only", "Lower Case Only"]

        n = buttons.length

        @validLbl = Qt::Label.new("", page)
        @validLbl.setGeometry(x, y + 50, 250, 20)
        @validLbl.show()

#        grp = KDE::ButtonGroup.new(n, Qt::Vertical, "Select valid chars", page)
        grp = KDE::ButtonGroup.new(page)
        grp.setGeometry(x, y + 75, 150, 5+30*n)
		
		(0...n).each { |i| Qt::RadioButton.new(buttons[i], grp) }

        connect(grp, SIGNAL("clicked(int)"), SLOT('restrict(int)'))

#        grp.find(0).setChecked(true)
#        box = grp.find(0)
#        box.setChecked(true)
        restrict(0)

        grp.show()

        page.show()
        $kapp.processEvents()

        y = y + 195
        sqzLbl = Qt::Label.new("This text is too long to fit in the label below", page)
        sqzLbl.setGeometry(x, y, 350, 20)
        sqzLbl.show()

        sqzLbl1 = Qt::Label.new("KSqueezedTxtLabel:", page)
        sqzLbl1.setGeometry(x, y + 20, 120, 20)
        sqzLbl1.show()

        squeeze = KDE::SqueezedTextLabel.new("This text is too long to fit in the label below", page)
        squeeze.setGeometry(x + 125, y + 20, 125, 20)
#        squeeze.setBackgroundMode(Qt::Widget::PaletteBase)
        squeeze.show()
	end

    def restrict(which)
        r = {0 => "0123456789", 1 => "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 2 => "abcdefghijklmnopqrstuvwxyz"}
        @restrictlineedit.setValidChars(r[which])
        @validLbl.setText("Valid: " + @restrictlineedit.validChars())
	end
end

class Page2
    def initialize(parent)
        page = parent.addPage()

        x1 = 10
        y1 = 10
        x2 = 240
        y2 = 100

        cbLbl = Qt::Label.new("KComboBox", page)
        cbLbl.setGeometry(x1, y1, 75, 20)
        cbLbl.show()

        combo = KDE::ComboBox.new(page)
        combo.addItems(["One", "Two", "Three"])
        combo.setGeometry(x1, y1 + 20, 100, 25)
        combo.show()

        ccbLbl = Qt::Label.new("KColorCombo", page)
        ccbLbl.setGeometry(x2, y1, 100, 20)
        ccbLbl.show()

        colorCombo = KDE::ColorCombo.new(page)
        colorCombo.setGeometry(x2, y1 + 20, 100, 25)
        colorCombo.show()

        editListBox = KDE::EditListBox.new("KEditListBox", page)
        editListBox.setGeometry(x1, y2, 220, 175)
        editListBox.insertStringList(["One", "Two", "Three"])
        editListBox.show()

        lbLbl = Qt::Label.new("KListWidget", page)
        lbLbl.setGeometry(x2, y2, 100, 20)
        lbLbl.show()

        listBox = KDE::ListWidget.new(page)
        listBox.setGeometry(x2, y2 + 20, 100, 100)
        listBox.addItems(["One", "Two", "Three"])
        listBox.show()
	end
end

class Page3
    def initialize(parent)
        page = parent.addPage()

        x = 10
        y = 10

        fontLbl   = Qt::Label.new("KFontChooser", page)
        fontLbl.setGeometry(x, y, 95, 20)
        fontLbl.show()

        fontChoose = KDE::FontChooser.new(page)
        fontChoose.setGeometry(x, y + 20, 375, 300)
        fontChoose.show()

        y = y + 330
	end
end

class Page4
    def initialize(parent)
        page = parent.addPage()

        x = 10
        y = 10

        cbLbl   = Qt::Label.new("KColorButton", page)
        cbLbl.setGeometry(x, y, 75, 20)
        cbLbl.show()

        cb = KDE::ColorButton.new(page)
        cb.setColor(Qt::Color.new(255, 0, 0))
        cb.setGeometry(x, y + 20, 30, 30)
        cb.show()

        ccbLbl = Qt::Label.new("KColorCombo", page)
        ccbLbl.setGeometry(x + 150, y, 100, 20)
        ccbLbl.show()

        colorCombo = KDE::ColorCombo.new(page)
        colorCombo.setGeometry(x + 150, y + 20, 100, 25)
        colorCombo.show()

        y = y + 60

        cpLbl   = Qt::Label.new("KColorPatch", page)
        cpLbl.setGeometry(x, y, 75, 20)
        cpLbl.show()

        cp = KDE::ColorPatch.new(page)
        cp.setColor(Qt::Color.new(255, 0, 0))
        cp.setGeometry(x, y + 20, 20, 20)
        cp.show()

        x = x + 150

        ccLbl   = Qt::Label.new("KColorCells", page)
        ccLbl.setGeometry(x, y, 75, 20)
        ccLbl.show()

        cc = KDE::ColorCells.new(page, 1, 5)
        cc.setColor(0, Qt::Color.new(0, 0, 0))
        cc.setColor(1, Qt::Color.new(255, 0, 0))
        cc.setColor(2, Qt::Color.new(0, 255, 0))
        cc.setColor(3, Qt::Color.new(0, 0, 255))
        cc.setColor(4, Qt::Color.new(255, 255, 255))
        cc.setGeometry(x, y + 20, 100, 20)
        cc.show()

        x = 10
        y = y + 50

        gsLbl   = Qt::Label.new("KGradientSelector", page)
        gsLbl.setGeometry(x + 80, y + 30, 110, 20)
        gsLbl.show()

        gs = KDE::GradientSelector.new(page)
        gs.setGeometry(x + 80, y + 50, 250, 20)
        gs.setColors(Qt::Color.new(255, 0, 0), Qt::Color.new(255, 255, 0))
        gs.show()

        y = y + 80

        hsLbl   = Qt::Label.new("KHueSaturationSelector", page)
        hsLbl.setGeometry(x, y, 95, 20)
        hsLbl.show()

        hs = KDE::HueSaturationSelector.new(page)
        hs.setGeometry(x, y + 20, 350, 80)
        hs.show()

        y = y + 110

#        ptLbl   = Qt::Label.new("KColorTable", page)
#        ptLbl.setGeometry(x, y, 95, 20)
#        ptLbl.show()

#        pt = KDE::ColorTable.new(page, 340, 24)
#        pt.setPalette("Royal")
#        pt.setGeometry(x, y + 20, 340, 40)
#        pt.show()
	end
end

class Page5
    def initialize(parent)
        page = parent.addPage()

        x = 10
        y = 10

#        rpLbl   = Qt::Label.new("KRootPermsIcon", page)
#        rpLbl.setGeometry(x, y, 95, 20)
#        rpLbl.show()

#        rp = KDE::RootPermsIcon.new(page)
#        rp.setGeometry(x, y + 20, 32, 32)
#        rp.show()

#        wpLbl   = Qt::Label.new("KWritePermsIcon", page)
#        wpLbl.setGeometry(x + 125, y, 95, 20)
#        wpLbl.show()

#        wp = KDE::WritePermsIcon.new("/usr/bin/gcc", page)
#        wp.setGeometry(x + 125, y + 20, 32, 32)
#        wp.show()

        y = y + 75

        pw1Lbl = Qt::Label.new("KLineEdit - echo *", page)
        pw1Lbl.setGeometry(x, y, 150, 20)
        pw1Lbl.show()

        pw1 = KDE::LineEdit.new("", page)
        pw1.passwordMode = true
        pw1.setGeometry(x, y + 20, 100, 20)
        pw1.show()

        y = y + 50

        pw2Lbl = Qt::Label.new("KPasswordEdit - echo ***", page)
        pw2Lbl.setGeometry(x, y, 150, 20)
        pw2Lbl.show()

        pw2 = KDE::LineEdit.new("", page)
        pw2.passwordMode = true
        pw2.setGeometry(x, y + 20, 100, 20)
        pw2.show()

        y = y + 50

        pw3Lbl = Qt::Label.new("KPasswordEdit - no echo", page)
        pw3Lbl.setGeometry(x, y, 150, 20)
        pw3Lbl.show()

        pw3 = KDE::LineEdit.new("", page)
        pw3.passwordMode = true
        pw3.setGeometry(x, y + 20, 100, 20)
        pw3.show()

        y = y + 50

        urlLbl = Qt::Label.new("KUrlLabel", page)
        urlLbl.setGeometry(x, y, 100, 20)
        urlLbl.show()

        url = KDE::UrlLabel.new("http://developer.kde.org/language-bindings/ruby/", "Korundum", page)
        url.setGeometry(x, y + 20, 100, 20)
        url.setUseTips(true)
        url.setTipText("http://developer.kde.org/language-bindings/ruby/")
        url.show()

        x = 70
        y = y + 50

        bbLbl   = Qt::Label.new("KDialogButtonBox", page)
        bbLbl.setGeometry(x, y, 75, 20)
        bbLbl.show()

        bbox = KDE::DialogButtonBox.new(page, Qt::Horizontal)
        bbox.setGeometry(x, y + 20, 300, 22)
        bbox.addButton("Button 1", Qt::DialogButtonBox::AcceptRole)
        bbox.addButton("Button 2", Qt::DialogButtonBox::AcceptRole)
        bbox.addButton("Button 3", Qt::DialogButtonBox::AcceptRole)
        bbox.show()

        y = y + 50

#        dbLbl   = Qt::Label.new("KDirectionButton", page)
#        dbLbl.setGeometry(x, y, 95, 20)
#        dbLbl.show()

#        dbUp    = KDE::DirectionButton.new(Qt::t::UpArrow, page)
#        dbDown  = KDE::DirectionButton.new(Qt::t::DownArrow, page)
#        dbRight = KDE::DirectionButton.new(Qt::t::RightArrow, page)
#        dbLeft  = KDE::DirectionButton.new(Qt::t::LeftArrow, page)

#        dbUp.setGeometry(x, y + 20, 22, 22)
#        dbDown.setGeometry(x + 30, y + 20, 22, 22)
#        dbRight.setGeometry(x + 60, y + 20, 22, 22)
#        dbLeft.setGeometry(x + 90, y + 20, 22, 22)

#        dbUp.show()
#        dbDown.show()
#        dbRight.show()
#        dbLeft.show()

        x = x + 150

#        kbLbl   = Qt::Label.new("KKeyButton", page)
#        kbLbl.setGeometry(x, y, 95, 20)
#        kbLbl.show()

#        kb = KDE::KeyButton.new(page)
#        kb.setText("Enter")
#        kb.setGeometry(x, y + 20, 50, 32)
#        kb.show()

        x = 70
        y = y + 50

#        tbLbl   = Qt::Label.new("KTabButton", page)
#        tbLbl.setGeometry(x, y, 95, 20)
#        tbLbl.show()

#        tbUp    = KDE::TabButton.new(Qt::t::UpArrow, page)
#        tbDown  = KDE::TabButton.new(Qt::t::DownArrow, page)
#        tbRight = KDE::TabButton.new(Qt::t::RightArrow, page)
#        tbLeft  = KDE::TabButton.new(Qt::t::LeftArrow, page)

#        tbUp.setGeometry(x, y + 20, 22, 25)
#        tbDown.setGeometry(x + 30, y + 20, 22, 25)
#        tbRight.setGeometry(x + 60, y + 20, 22, 25)
#        tbLeft.setGeometry(x + 90, y + 20, 22, 25)

#        tbUp.show()
#        tbDown.show()
#        tbRight.show()
#        tbLeft.show()
	end
end

class Page6 < Qt::Object
	slots 'toggleClicked()'
	
    def initialize(parent)
		super
        page = parent.addPage()

        x = 20
        y = 10

        red          = Qt::Color.new(255, 0, 0)
        green        = Qt::Color.new(0, 255, 0)
        yellow       = Qt::Color.new(255, 255, 0)
        blue         = Qt::Color.new(0, 0, 255)

        ledcolor     = [red, green, yellow, blue]
        ledshape     = [KDE::Led::Rectangular, KDE::Led::Circular]
        ledlook      = [KDE::Led::Flat, KDE::Led::Raised, KDE::Led::Sunken]
        ledsize      = [10, 18, 25]
        @ledlist = []

		ledlook.each do |look|
            ledcolor.each do |color|
                ledshape.each do |shape|
                    ledsize.each do |size|
                        led = KDE::Led.new(color, KDE::Led::On, look, shape, page)
                        led.setGeometry(x, y, size, size)
                        @ledlist << led
                        led.show()
                        x = x + 50
					end
                    x = x + 50
				end
                x = 20
                y = y + 30
			end
            y = y + 10
		end

        toggle = Qt::PushButton.new("Toggle", page)
        toggle.setGeometry(150, 400, 60, 22)
        toggle.show()

        connect(toggle, SIGNAL("clicked()"), SLOT('toggleClicked()'))

        page.show()
	end

    def toggleClicked()
		@ledlist.each { |led| led.toggle() }
	end
end

class Page7 < Qt::Object
	slots 'add1()'

    def initialize(parent)
		super
        page = parent.addPage()

        x = 10
        y = 10

=begin
        tabLbl   = Qt::Label.new("KTabCtl", page)
        tabLbl.setGeometry(x, y, 95, 20)
        tabLbl.show()

        tab = KDE::TabCtl.new(page)
        tab.setGeometry(x, y + 20, 300, 100)

        page1 = Qt::Widget.new(tab)
        p1Lbl = Qt::Label.new("This is page 1", page1)
        p1Lbl.setGeometry(20, 20, 100, 20)
        page2 = Qt::Widget.new(tab)
        p2Lbl = Qt::Label.new("This is page 2", page2)
        p2Lbl.setGeometry(50, 20, 100, 20)
        page3 = Qt::Widget.new(tab)
        p3Lbl = Qt::Label.new("This is page 3", page3)
        p3Lbl.setGeometry(20, 50, 100, 20)

        tab.addTab(page1, "Tab 1")
        tab.addTab(page2, "Tab 2")
        tab.addTab(page3, "Tab 3")
        tab.show()
=end
        x = 10
        y = 150
=begin
        progLbl   = Qt::Label.new("KProgress", page)
        progLbl.setGeometry(x, y + 50, 95, 20)
        progLbl.show()

        @p1 = KDE::Progress.new(page)
        @p2 = KDE::Progress.new(15, page)
        @p1.setTotalSteps(25)
        @p2.setTotalSteps(25)

        @p1.setGeometry(x, y + 80, 125, 20)
        @p2.setGeometry(x, y + 120, 125, 20)

        @p2.setPercentageVisible(0)

        @p1.show()
        @p2.show()
=end
        @total = 0

        y = y + 150
        sepLbl   = Qt::Label.new("KSeparator", page)
        sepLbl.setGeometry(x, y, 95, 20)
        sepLbl.show()

        sep = KDE::Separator.new(Qt::Frame::HLine, page)
        sep.setGeometry(x, y + 20, 75, 10)
        sep.show()

        page.show()

        @timer = Qt::Timer.new(page)
		connect(@timer, SIGNAL('timeout()'), SLOT('add1()'))
		@timer.start(100)

        add1()
	end

    def add1()
        @total = @total + 1
#        @p1.advance(1)
#        @p2.advance(1)

        if @total == 26
			@timer.stop
		end
	end
end

class Page8
    def initialize(parent)
        page = parent.addPage()

        x = 40
        y = 10

        dpLbl   = Qt::Label.new("KDatePicker", page)
        dpLbl.setGeometry(x, y, 95, 20)
        dpLbl.show()

        dp = KDE::DatePicker.new(page)
        dp.setGeometry(x, y + 20, 300, 170)
        dp.show()

        y = y + 210

        dtLbl   = Qt::Label.new("KDateTable", page)
        dtLbl.setGeometry(x, y, 95, 20)
        dtLbl.show()

        dt = KDE::DateTable.new(page)
        dt.setGeometry(x, y + 20, 300, 130)
        dt.show()
	end
end

class PageThisApp
    def initialize(parent)
        page = parent.addPage()

        x = 10
        y = 10

        taLbl   = Qt::Label.new("This application uses KMainWindow as its top level widget\n and KTreeView in the"\
                          " left-hand panel", page)
        taLbl.setGeometry(x, y, 300, 60)
        taLbl.show()
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

class CSDlg < KDE::Dialog
	slots 'closeClicked()'
	
    def initialize(parent)
        super(parent)

        setGeometry(150, 50, 700, 320)
        x = 10
        y = 10

        csLbl   = Qt::Label.new("KCharSelect", self)
        csLbl.setGeometry(x, y, 95, 20)
        csLbl.show()
        cs = KDE::CharSelect.new(self, nil)
        cs.setGeometry(x, y + 20, 680, 250)
        cs.show()

        closeBtn = Qt::PushButton.new("Close", self)
        closeBtn.setGeometry( 610, 280, 60, 22)
        closeBtn.show()

        connect(closeBtn, SIGNAL("clicked()"), SLOT('closeClicked()'))
	end

    def closeClicked()
        done(1)
	end
end

def UIWidgets.widKAnimWidget(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKAuthIcon(parent)
    parent.currentPageObj = Page5.new(parent)
end

def UIWidgets.widKButtonBox(parent)
    parent.currentPageObj = Page5.new(parent)
end

def UIWidgets.widKCharSelect(parent)
    dlg = CSDlg.new(parent)
    dlg.show()
end

def UIWidgets.widKColorButton(parent)
    parent.currentPageObj = Page4.new(parent)
end

def UIWidgets.widKColorCells(parent)
    parent.currentPageObj = Page4.new(parent)
end

def UIWidgets.widKColorCombo(parent)
    parent.currentPageObj = Page2.new(parent)
end

def UIWidgets.widKColorPatch(parent)
    parent.currentPageObj = Page4.new(parent)
end

def UIWidgets.widKComboBox(parent)
    parent.currentPageObj = Page2.new(parent)
end

def UIWidgets.widKCompletionBox(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKContainerLayout(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKCursor(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKDatePicker(parent)
    parent.currentPageObj = Page8.new(parent)
end

def UIWidgets.widKDateTable(parent)
    parent.currentPageObj = Page8.new(parent)
end

def UIWidgets.widKDirectionButton(parent)
    parent.currentPageObj = Page5.new(parent)
end

def UIWidgets.widKDualColorButton(parent)
    parent.currentPageObj = Page4.new(parent)
end

def UIWidgets.widKTextEdit(parent)
    parent.currentPageObj = Page1.new(parent)
end

def UIWidgets.widKEditListBox(parent)
    parent.currentPageObj = Page2.new(parent)
end

def UIWidgets.widKFontChooser(parent)
    parent.currentPageObj = Page3.new(parent)
end

def UIWidgets.widKHueSaturationSelector(parent)
    parent.currentPageObj = Page4.new(parent)
end

def UIWidgets.widKIconView(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKJanusWidget(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

#def UIWidgets.widKKeyButton(parent)
#    parent.currentPageObj = Page5.new(parent)

def UIWidgets.widKKeyChooser(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKLed(parent)
    parent.currentPageObj = Page6.new(parent)
end

def UIWidgets.widKLineEdit(parent)
    parent.currentPageObj = Page1.new(parent)
end

def UIWidgets.widKListBox(parent)
    parent.currentPageObj = Page2.new(parent)
end

def UIWidgets.widKTreeWidget(parent)
    parent.currentPageObj = PageThisApp.new(parent)
end

def UIWidgets.widKNumInput(parent)
    parent.currentPageObj = Page1.new(parent)
end

def UIWidgets.widKColorTable(parent)
    parent.currentPageObj = Page4.new(parent)
end

def UIWidgets.widKPasswordEdit(parent)
    parent.currentPageObj = Page5.new(parent)
end

def UIWidgets.widKProgress(parent)
    parent.currentPageObj = Page7.new(parent)
end

def UIWidgets.widKRootPixmap(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKMainWindow(parent)
    parent.currentPageObj = PageThisApp.new(parent)
end

def UIWidgets.widKRestrictedLine(parent)
    parent.currentPageObj = Page1.new(parent)
end

def UIWidgets.widKRuler(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKSelector(parent)
    parent.currentPageObj = Page4.new(parent)
end

def UIWidgets.widKSeparator(parent)
    parent.currentPageObj = Page7.new(parent)
end

def UIWidgets.widKSqueezedTextLabel(parent)
    parent.currentPageObj = Page1.new(parent)
end

def UIWidgets.widKTabButton(parent)
    parent.currentPageObj = Page5.new(parent)
end

def UIWidgets.widKTabCtl(parent)
    parent.currentPageObj = Page7.new(parent)
end

def UIWidgets.widKTextBrowser(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIWidgets.widKUrlLabel(parent)
    parent.currentPageObj = Page5.new(parent)
end


if $0 == __FILE__
    puts
    puts "Please run uisampler.rb"
    puts
end

end
