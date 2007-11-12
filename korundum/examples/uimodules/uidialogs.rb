=begin
This is a ruby version of Jim Bublitz's pykde program, translated by Richard Dale
=end

module UIDialogs

class CustomDlg < KDE::Dialog

	slots 'dlgClicked()', 'okClicked()', 'cancelClicked()'
	
    def initialize(parent)
        super(parent)

        x = 20
        y = 10

        rLbl        = Qt::Label.new("r", self)
        gLbl        = Qt::Label.new("g", self)
        bLbl        = Qt::Label.new("b", self)
        @rEd    = Qt::LineEdit.new("64", self)
        @gEd    = Qt::LineEdit.new("64", self)
        @bEd    = Qt::LineEdit.new("64", self)
        dlgBtn = Qt::PushButton.new("Set/Get Color", self)
        okBtn  = Qt::PushButton.new("OK", self)
        canBtn = Qt::PushButton.new("Cancel", self)

        rLbl.setGeometry(x, y, 25, 20)
        gLbl.setGeometry(x + 30, y, 25, 20)
        bLbl.setGeometry(x + 60, y, 25, 20)
        y = y + 20
        @rEd.setGeometry(x, y, 25, 20)
        @gEd.setGeometry(x + 30, y, 25, 20)
        @bEd.setGeometry(x + 60, y, 25, 20)
        y = y + 30
        dlgBtn.setGeometry(x, y, 90, 22)
        y = y + 30
        okBtn.setGeometry(x, y, 40, 22)
        canBtn.setGeometry(x + 50, y, 40, 22)

        connect(dlgBtn, SIGNAL("clicked()"), SLOT('dlgClicked()'))
        connect(okBtn, SIGNAL("clicked()"), SLOT('okClicked()'))
        connect(canBtn, SIGNAL("clicked()"), SLOT('cancelClicked()'))
	end
    
	def dlgClicked()
        # get some(numerical) color values from the original dialog
        red   = @rEd.text().to_i
        green = @gEd.text().to_i
        blue  = @bEd.text().to_i

        # convert the numbers to a Qt::Color
        color = Qt::Color.new(red, green, blue)

        # invoke the dialog(getColor is a 'static' call)
        # initialize with the colors from above(in color)
        # color will also hold the new value chosen in the
        # KDE::ColorDialog
        result = KDE::ColorDialog.getColor(color, self)

        # get the numerical color values back
#        red, green, blue = color.rgb()

        # update the Qt::LineEdits in the original dialog
        @rEd.setText(red.to_s)
        @gEd.setText(green.to_s)
        @bEd.setText(blue.to_s)
	end

    def okClicked()
        done(1)
	end

    def cancelClicked()
        done(0)
	end
end

class MessageDlg < KDE::Dialog

	slots 'launch(int)'
	
    def initialize(parent)
        super(parent)

        buttons = ["QuestionYesNo", "WarningYesNo", "WarningContinueCancel", "WarningYesNoCancel",
                   "Information", "SSLMessageBox", "Sorry", "Error", "QuestionYesNoCancel"]

        setGeometry(10, 10, 500, 65 * buttons.length)

        grp = Qt::ButtonGroup.new(self)
        widget = Qt::GroupBox.new("MessageBoxes", self)
        widget.setGeometry(10, 10, 260, 60 * buttons.length)
        vlayout = Qt::VBoxLayout.new(widget)
		buttons.each_index do |i|
            button = Qt::RadioButton.new(buttons[i])
            grp.addButton(button, i)
            vlayout.addWidget(button)
        end
        widget.show
        connect(grp, SIGNAL("buttonClicked(int)"), SLOT('launch(int)'))
	end

    def launch(which)
		which += 1 # Qt::ButtonGroup id's start at 0, but the KDE::MessageBox enum starts at 1
		
        if which == KDE::MessageBox::QuestionYesNo
            KDE::MessageBox.questionYesNo(self, "This is a questionYesNo message box\nThere is also a list version of this dialog",\
                                       "questionYesNo")

        elsif which == KDE::MessageBox::WarningYesNo
            KDE::MessageBox.warningYesNo(self, "This is a warningYesNo message box", "warningYesNo")

        elsif which == KDE::MessageBox::WarningContinueCancel
            KDE::MessageBox.warningContinueCancel(self, "This is a warningContinueCancel message box", "warningContinueCancel");

        elsif which == KDE::MessageBox::WarningYesNoCancel
            KDE::MessageBox.warningYesNoCancel(self, "This is a warningYesNoCancel message box", "warningYesNoCancel")

        elsif which == KDE::MessageBox::Information
            KDE::MessageBox.information(self, "This is an information message box", "Information")

#        elsif which == KDE::MessageBox::SSLMessageBox
#            KDE::MessageBox.SSLMessageBox(self, "This is an SSLMessageBox message box", "not implemented yet")

        elsif which == KDE::MessageBox::Sorry
            KDE::MessageBox.sorry(self, "This is a 'sorry' message box", "Sorry")

        elsif which == KDE::MessageBox::Error
            KDE::MessageBox.error(self, "No - this isn't really an error\nIt's an error message box\n", "Error")
        
		elsif which == KDE::MessageBox::QuestionYesNoCancel
            KDE::MessageBox.questionYesNoCancel(self, "No - this isn't really an error\nIt's an QuestionYesNoCancel message box\n", "QuestionYesNoCancel")
		end
	end
end


def UIDialogs.dlgKAboutDialog(parent)
    dlg = KDE::AboutApplicationDialog.new(parent, 'about dialog', false)
    dlg.setLogo(Qt::Pixmap.new("rbtestimage.png"))
    dlg.setTitle("UISampler for Korundum")
    dlg.setAuthor("Jim Bublitz", "jbublitz@nwinternet.com", "http://www.riverbankcomputing.co.uk",
                    "\n\nPyKDE -- Python bindings\n\tfor KDE")
    dlg.setMaintainer("Richard Dale", "Richard_Dale@tipitina.demon.co.uk", "http://developer.kde.org/language-bindings/ruby/",\
                    "\n\nKorundum -- Ruby bindings\n\tfor KDE")
    dlg.addContributor("KDE bindings list", "kde-bindings@kde.org", nil, nil)

    dlg.show()
end


def UIDialogs.dlgKBugReport(parent)
    dlg = KDE::BugReport.new(parent)
    dlg.exec()
end

def UIDialogs.dlgKAboutApplicationDialog(parent)
    about = KDE::AboutData.new("uisampler", "UI Sampler", KDE.ki18n("KDE UI Widgets Sampler"), "0.1")
    dlg = KDE::AboutApplicationDialog.new(about, parent)
    dlg.show()
end

def UIDialogs.dlgKColorDialog(parent)
    dlg = KDE::ColorDialog.new(parent, false)
    dlg.show()
end

def UIDialogs.dlgKDialog(parent)
    dlg = CustomDlg.new(parent)
    dlg.show()
end

def UIDialogs.dlgKDialogBase(parent)
    caption = "KDialogBase sample"
    text_ = "This is a KDialogBase example"
    dlg =   KDE::DialogBase.new(parent, false, caption,
            KDE::DialogBase::Ok | KDE::DialogBase::Cancel, KDE::DialogBase::Ok, true )

    page  = dlg.makeVBoxMainWidget();

    # making 'page' the parent inserts the widgets in
    # the VBox created above
    label = Qt::Label.new( caption, page, "caption" );

    lineedit = Qt::LineEdit.new(text_, page, "lineedit" );
    lineedit.setMinimumWidth(dlg.fontMetrics().maxWidth()*20);

    label0 = Qt::Label.new("Border widths", page)
#    widths = dlg.getBorderWidths()
#    labelA = Qt::Label.new("Upper Left X: " + widths[0].to_s, page)
#    labelB = Qt::Label.new("Upper Left Y: " + widths[0].to_s, page)
#    labelC = Qt::Label.new("Lower Right X: " + str(c), page)
#    labelD = Qt::Label.new("Lower Right Y: " + str(d), page)

    dlg.show()
end

def UIDialogs.dlgKFontDialog(parent)
    dlg = KDE::FontDialog.new(parent, "font dlg", false, false)
    dlg.show()
end

def UIDialogs.dlgKShortcutsDialog(parent)
    # This really doesn't do anything except pop up the dlg
#    keys = KDE::Accel.new(parent)
#    keys.insertItem( i18n( "Zoom in" ), "Zoom in", "+" )
#    keys.readSettings();
    dlg = KDE::ShortcutsDialog.new
    dlg.show()
end

def UIDialogs.dlgKLineEditDlg(parent)
#    result, ok = KDE::LineEditDlg.getText("Enter text", "<Your input here>", parent)
#    print "result", result
#    print "ok", ok

    # pop up another dlg to show what happened in the KDE::LineEditDlg
#    if ok
#        result = result.latin1()
#        KDE::MessageBox.information(parent, "OK was pressed\nText: " + result, "KDE::LineEditDlg result")
#    else:
#        result = ""
#        KDE::MessageBox.information(parent, "Cancel pressed\nText: " + result, "KDE::LineEditDlg result")
end

def UIDialogs.dlgKMessageBox(parent)
    dlg = MessageDlg.new(parent)
    dlg.show()
end

def UIDialogs.dlgKPasswordDialog(parent)
	password = ""
    result = KDE::PasswordDialog.getPassword(password, "Enter password(just a test)")
	puts "password: #{password}"
end

def UIDialogs.dlgKWizard(parent)
    wiz = KDE::Wizard.new(parent)

    page1 = Qt::Widget.new(wiz)
    p1Lbl = Qt::Label.new("This is page 1", page1)
    p1Lbl.setGeometry(20, 20, 100, 20)
    page2 = Qt::Widget.new(wiz)
    p2Lbl = Qt::Label.new("This is page 2", page2)
    p2Lbl.setGeometry(50, 20, 100, 20)
    page3 = Qt::Widget.new(wiz)
    p3Lbl = Qt::Label.new("This is page 3", page3)
    p3Lbl.setGeometry(80, 20, 100, 20)

    wiz.addPage(page1, "Page 1")
    wiz.addPage(page2, "Page 2")
    wiz.addPage(page3, "Page 3")
    wiz.show()
end

if $0 == __FILE__
    puts
    puts "Please run uisampler.rb"
    puts
end

end
