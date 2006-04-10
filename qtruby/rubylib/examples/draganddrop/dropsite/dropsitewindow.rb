=begin
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**

** Translated to QtRuby by Richard Dale
=end
		
require 'dropsitewidget.rb'
	
class DropSiteWindow < Qt::Widget
	
	slots 'updateSupportedFormats(const QMimeData *)'
	
	def initialize(parent = nil)
	    super(parent)
	    @abstractLabel = Qt::Label.new(tr("The Drop Site example accepts drops from other " \
	                                  "applications, and displays the MIME formats " \
	                                  "provided by the drag object."))
	    @abstractLabel.wordWrap = true
	    @abstractLabel.adjustSize()
	
	    @dropSiteWidget = DropSiteWidget.new
	    connect(@dropSiteWidget, SIGNAL('changed(const QMimeData*)'),
	            self, SLOT('updateSupportedFormats(const QMimeData*)'))
	
	    @supportedFormats = Qt::TableWidget.new(0, 2)
	    labels = []
	    labels << tr("Format") << tr("Content")
	    @supportedFormats.horizontalHeaderLabels = labels
	    @supportedFormats.horizontalHeader().stretchLastSection = true
	
	    @quitButton = Qt::PushButton.new(tr("Quit"))
	    connect(@quitButton, SIGNAL('pressed()'), self, SLOT('close()'))
	
	    @clearButton = Qt::PushButton.new(tr("Clear"))
	    connect(@clearButton, SIGNAL('pressed()'), @dropSiteWidget, SLOT('clear()'))
	
	    buttonLayout = Qt::HBoxLayout.new do |b|
			b.addStretch()
			b.addWidget(@clearButton)
			b.addWidget(@quitButton)
			b.addStretch()
		end

	    @layout = Qt::VBoxLayout.new do |l|
			l.addWidget(@abstractLabel)
			l.addWidget(@dropSiteWidget)
			l.addWidget(@supportedFormats)
			l.addLayout(buttonLayout)
		end
	
	    setLayout(@layout)
	    setMinimumSize(350, 500)
	    setWindowTitle(tr("Drop Site"))
	end
	
	def resizeEvent(event)
	    @supportedFormats.resizeColumnToContents(0)
	    super(event)
	end
	
	def updateSupportedFormats(mimeData = nil)
	    @supportedFormats.rowCount = 0
	
	    if mimeData.nil?
	        return
		end
	
	    formats = mimeData.formats()

	    formats.each do |format|
	        formatItem = Qt::TableWidgetItem.new(format)
	        formatItem.flags = Qt::ItemIsEnabled
	        formatItem.textAlignment = Qt::AlignTop | Qt::AlignLeft
	
	        data = mimeData.data(format)
	
	        text = @dropSiteWidget.createPlainText(data, format)
	        $qApp.processEvents()
	        if !text.empty?
	            dataItem = Qt::TableWidgetItem.new(text)
	        else
	            hexdata = ""
                data.to_s.each_byte { |b| hexdata << ("%2.2x " % b) }
	            dataItem = Qt::TableWidgetItem.new(hexdata)
	        end
	        dataItem.flags = Qt::ItemIsEnabled
	
	        row = @supportedFormats.rowCount()
	        @supportedFormats.insertRow(row)
	        @supportedFormats.setItem(row, 0, formatItem)
	        @supportedFormats.setItem(row, 1, dataItem)
	    end
	
	    @supportedFormats.resizeColumnToContents(0)
	end
end
