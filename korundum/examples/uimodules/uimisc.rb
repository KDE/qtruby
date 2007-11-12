=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

module UIMisc

class Page3 < Qt::Object
	slots 'ivChanged()', 'fvChanged()', 'dvChanged()'
	
    def initialize(parent)
		super
        page = parent.addPage()
        x = 10
        y = 15

        green  = Qt::Color.new(0, 255, 0)
        yellow = Qt::Color.new(255, 255, 0)
        red    = Qt::Color.new(255, 0, 0)

        ivLbl  = Qt::Label.new("KIntValidator", page)
        ivLbl.setGeometry(x, y, 100, 20)
        ivLbl.show()

        @iv = KDE::LineEdit.new(page)
        @iv.setGeometry(x, y + 20, 100, 20)
        @iv.show()
        connect(@iv, SIGNAL("textChanged(const QString&)"), SLOT('ivChanged()'))

        @ivVal = KDE::IntValidator.new(page)
        @ivVal.setRange(20, 50)

        ivRngLbl = Qt::Label.new("Range is 20 - 50", page)
        ivRngLbl.setGeometry(x, y + 45, 100, 20)
        ivRngLbl.show()

        ivAccLbl   = Qt::Label.new("Acceptable", page)
        ivAccLbl.setGeometry(x + 125, y + 45, 85, 20)
        ivAccLbl.show()
        ivInterLbl = Qt::Label.new("Intermediate", page)
        ivInterLbl.setGeometry(x + 125, y + 20, 85, 20)
        ivInterLbl.show()
        ivInvalLbl = Qt::Label.new("Invalid", page)
        ivInvalLbl.setGeometry(x + 125, y - 5, 85, 20)
        ivInvalLbl.show()
        @ivInvalLed = KDE::Led.new(red, KDE::Led::Off, KDE::Led::Sunken, KDE::Led::Circular,page)
        @ivInvalLed.setGeometry(x + 215, y - 5, 18, 18)
        @ivInvalLed.show()
        @ivInterLed = KDE::Led.new(yellow, KDE::Led::Off, KDE::Led::Sunken, KDE::Led::Circular,page)
        @ivInterLed.setGeometry(x + 215, y + 20, 18, 18)
        @ivInterLed.show()
        @ivAccLed = KDE::Led.new(green, KDE::Led::On, KDE::Led::Sunken, KDE::Led::Circular,page)
        @ivAccLed.setGeometry(x + 215, y + 45, 18, 18)
        @ivAccLed.show()

        y = y + 100

        fvLbl  = Qt::Label.new("KDoubleValidator", page)
        fvLbl.setGeometry(x, y, 100, 20)
        fvLbl.show()

        @fv = KDE::LineEdit.new(page)
        @fv.setGeometry(x, y + 20, 100, 20)
        @fv.show()
        connect(@fv, SIGNAL("textChanged(const QString&)"), SLOT('fvChanged()'))

        @fvVal = KDE::DoubleValidator.new(page)
        @fvVal.setRange(10.0, 40.0)

        fvRngLbl = Qt::Label.new("Range is 10.0 - 40.0", page)
        fvRngLbl.setGeometry(x, y + 45, 100, 20)
        fvRngLbl.show()

        fvAccLbl   = Qt::Label.new("Acceptable", page)
        fvAccLbl.setGeometry(x + 125, y + 45, 85, 20)
        fvAccLbl.show()
        fvInterLbl = Qt::Label.new("Intermediate", page)
        fvInterLbl.setGeometry(x + 125, y + 20, 95, 20)
        fvInterLbl.show()
        fvInvalLbl = Qt::Label.new("Invalid", page)
        fvInvalLbl.setGeometry(x + 125, y - 5, 85, 20)
        fvInvalLbl.show()
        @fvInvalLed = KDE::Led.new(red, KDE::Led::Off, KDE::Led::Sunken, KDE::Led::Circular,page)
        @fvInvalLed.setGeometry(x + 215, y - 5, 18, 18)
        @fvInvalLed.show()
        @fvInterLed = KDE::Led.new(yellow, KDE::Led::Off, KDE::Led::Sunken, KDE::Led::Circular,page)
        @fvInterLed.setGeometry(x + 215, y + 20, 18, 18)
        @fvInterLed.show()
        @fvAccLed = KDE::Led.new(green, KDE::Led::On, KDE::Led::Sunken, KDE::Led::Circular,page)
        @fvAccLed.setGeometry(x + 215, y + 45, 18, 18)
        @fvAccLed.show()

        y = y + 100

        dvLbl  = Qt::Label.new("KDateValidator", page)
        dvLbl.setGeometry(x, y, 100, 20)
        dvLbl.show()

        @dv = KDE::LineEdit.new(page)
        @dv.setGeometry(x, y + 20, 100, 20)
        @dv.show()
#        connect(dv, SIGNAL("textChanged(const QString&)"), SLOT('dvChanged()'))

        @dvVal = KDE::DateValidator.new(page)
#        dvVal.setRange(10.0, 40.0)

#        dvRngLbl = Qt::Label.new("Range is 10.0 - 40.0", page)
#        dvRngLbl.setGeometry(x, y + 45, 100, 20)
#        dvRngLbl.show()

        dvBtn = Qt::PushButton.new("Validate", page)
        dvBtn.setGeometry(x, y + 45, 60, 22)
        dvBtn.show()
        connect(dvBtn, SIGNAL("clicked()"), SLOT('dvChanged()'))

        dvNoteLbl = Qt::Label.new("Format is locale dependent\nShort date only\nTry DD-MM-YY", page)
        dvNoteLbl.setGeometry(x, y + 70, 150, 60)
        dvNoteLbl.show()

        dvAccLbl   = Qt::Label.new("Acceptable", page)
        dvAccLbl.setGeometry(x + 125, y + 45, 85, 20)
        dvAccLbl.show()
        dvInterLbl = Qt::Label.new("Intermediate", page)
        dvInterLbl.setGeometry(x + 125, y + 20, 85, 20)
        dvInterLbl.show()
        dvInvalLbl = Qt::Label.new("Invalid", page)
        dvInvalLbl.setGeometry(x + 125, y - 5, 85, 20)
        dvInvalLbl.show()
        @dvInvalLed = KDE::Led.new(red, KDE::Led::Off, KDE::Led::Sunken, KDE::Led::Circular,page)
        @dvInvalLed.setGeometry(x + 215, y - 5, 18, 18)
        @dvInvalLed.show()
        @dvInterLed = KDE::Led.new(yellow, KDE::Led::Off, KDE::Led::Sunken, KDE::Led::Circular,page)
        @dvInterLed.setGeometry(x + 215, y + 20, 18, 18)
        @dvInterLed.show()
        @dvAccLed = KDE::Led.new(green, KDE::Led::On, KDE::Led::Sunken, KDE::Led::Circular,page)
        @dvAccLed.setGeometry(x + 215, y + 45, 18, 18)
        @dvAccLed.show()
	end

    def ivChanged()
        @ivInvalLed.off()
        @ivInterLed.off()
        @ivAccLed.off()

		i = Qt::Integer.new(0)
        state = @ivVal.validate(@iv.text(), i)

        if state == Qt::Validator::Acceptable
            @ivAccLed.on()
        elsif state == Qt::Validator::Intermediate
            @ivInterLed.on()
        else
            @ivInvalLed.on()
		end
	end

    def fvChanged()
        @fvInvalLed.off()
        @fvInterLed.off()
        @fvAccLed.off()

		i = Qt::Integer.new(0)
        state = @fvVal.validate(@fv.text(), i)

        if state == Qt::Validator::Acceptable
            @fvAccLed.on()
        elsif state == Qt::Validator::Intermediate
            @fvInterLed.on()
        else
            @fvInvalLed.on()
		end
	end

    def dvChanged()
        @dvInvalLed.off()
        @dvInterLed.off()
        @dvAccLed.off()

		i = Qt::Integer.new(0)
        state = @dvVal.validate(@dv.text(), i)

        if state == Qt::Validator::Acceptable
            @dvAccLed.on()
        elsif state == Qt::Validator::Intermediate
            @dvInterLed.on()
        else
            @dvInvalLed.on()
		end
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

def UIMisc.miscKAlphaPainter(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKCModule(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKColor(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKColorDrag(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKCommand(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKCommandHistory(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKDockWindow(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKDoubleValidator(parent)
    parent.currentPageObj = Page3.new(parent)
end

def UIMisc.miscKDateValidator(parent)
    parent.currentPageObj = Page3.new(parent)
end

def UIMisc.miscKIntValidator(parent)
    parent.currentPageObj = Page3.new(parent)
end

def UIMisc.miscKPixmapIO(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKSharedPixmap(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscKSystemTray(parent)
    KDE::MessageBox.information(parent, "See the systray.rb example in the templates/ subdirectories")
end

def UIMisc.miscKThemeBase(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

def UIMisc.miscQXEmbed(parent)
    parent.currentPageObj = PageNotImpl.new(parent)
end

if $0 == __FILE__
    puts
    puts "Please run uisampler.rb"
    puts
end

end

