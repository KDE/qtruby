# Form implementation generated from reading ui file 'calculatorform.ui'
#
# Created: Sun Sep 18 13:46:48 2005
#      by: The QtRuby User Interface Compiler (rbuic)
#
# WARNING! All changes made in this file will be lost!


require 'Qt'

class CalculatorForm < Qt::Widget



    def initialize(parent = nil, name = nil, fl = 0)
        super

        setProperty("sizePolicy", Qt::Variant.new(Qt::SizePolicy.new(5, 5, 0, 0, self.sizePolicy().hasHeightForWidth())))

        languageChange()
        resize( Qt::Size.new(400, 300).expandedTo(minimumSizeHint()) )
        clearWState( WState_Polished )

    end

    #
    #  Sets the strings of the subwidgets using the current
    #  language.
    #
    def languageChange()
        setProperty("objectName", Qt::Variant.new(trUtf8("CalculatorForm")))
        setProperty("windowTitle", Qt::Variant.new(trUtf8("Calculator Form")))
    end
    protected :languageChange


end
