require 'wigglywidget.rb'

class Dialog < Qt::Dialog
    def initialize(parent = nil)
        super(parent)

        wigglyWidget = WigglyWidget.new
        lineEdit = Qt::LineEdit.new
    
        layout = Qt::VBoxLayout.new
        layout.addWidget(wigglyWidget)
        layout.addWidget(lineEdit)
        setLayout(layout)
    
        connect(lineEdit, SIGNAL('textChanged(QString)'),
                wigglyWidget, SLOT('setText(QString)'))
    
        lineEdit.setText(tr("Hello world!"))
    
        setWindowTitle(tr("Wiggly"))
        resize(360, 145)
    end
end