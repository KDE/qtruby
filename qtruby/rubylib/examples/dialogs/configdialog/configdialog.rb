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
	
require 'pages.rb'

class ConfigDialog < Qt::Dialog
	
	slots 'changePage(QListWidgetItem*, QListWidgetItem*)'
	
	def initialize()
		super
	    @contentsWidget = Qt::ListWidget.new do |c|
			c.viewMode = Qt::ListView::IconMode
			c.iconSize = Qt::Size.new(96, 84)
			c.movement = Qt::ListView::Static
			c.maximumWidth = 128
			c.spacing = 12
		end
	
	    @pagesWidget = Qt::StackedWidget.new do |p|
	    	p.addWidget(ConfigurationPage.new)
	    	p.addWidget(UpdatePage.new)
	    	p.addWidget(QueryPage.new)
		end
	
	    closeButton = Qt::PushButton.new(tr("Close"))
	
	    createIcons()
	    @contentsWidget.currentRow = 0
	
	    connect(closeButton, SIGNAL('clicked()'), self, SLOT('close()'))
	
	    horizontalLayout = Qt::HBoxLayout.new do |h|
	    	h.addWidget(@contentsWidget)
	    	h.addWidget(@pagesWidget, 1)
		end
	
	    buttonsLayout = Qt::HBoxLayout.new do |b|
	    	b.addStretch(1)
	    	b.addWidget(closeButton)
		end
	
	    self.layout = Qt::VBoxLayout.new do |m|
			m.addLayout(horizontalLayout)
			m.addStretch(1)
			m.addSpacing(12)
			m.addLayout(buttonsLayout)
		end
	
	    setWindowTitle(tr("Config Dialog"))
	end
	
	def createIcons()
	    configButton = Qt::ListWidgetItem.new(@contentsWidget)
	    configButton.icon = Qt::Icon.new("images/config.png")
	    configButton.text = tr("Configuration")
	    configButton.textAlignment = Qt::AlignHCenter
	    configButton.flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled
	
	    updateButton = Qt::ListWidgetItem.new(@contentsWidget)
	    updateButton.icon = Qt::Icon.new("images/update.png")
	    updateButton.text = tr("Update")
	    updateButton.textAlignment = Qt::AlignHCenter
	    updateButton.flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled
	
	    queryButton = Qt::ListWidgetItem.new(@contentsWidget)
	    queryButton.icon = Qt::Icon.new("images/query.png")
	    queryButton.text = tr("Query")
	    queryButton.textAlignment = Qt::AlignHCenter
	    queryButton.flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled
	
	    connect(@contentsWidget,
	            SIGNAL('currentItemChanged(QListWidgetItem*, QListWidgetItem*)'),
	            self, SLOT('changePage(QListWidgetItem*, QListWidgetItem*)'))
	end
	
	def changePage(current, previous)
	    if current.nil?
	        current = previous
		end

	    @pagesWidget.currentIndex = @contentsWidget.row(current)
	end
end
