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

require 'pixeldelegate.rb'
require 'imagemodel.rb'

class MainWindow < Qt::MainWindow
	
	slots	'chooseImage()',
    		'printImage()',
    		'showAboutBox()'
	
	def initialize()
		super
	    @currentPath = Qt::Dir.home.absolutePath
	    @model = nil
	
	    @view = Qt::TableView.new
	    @view.itemDelegate = PixelDelegate.new(self)
	    @view.showGrid = false
	    @view.horizontalHeader.hide
	    @view.verticalHeader.hide
	    setCentralWidget(@view)
	
	    fileMenu = Qt::Menu.new(tr("&File"), self)
	    openAction = fileMenu.addAction(tr("&Open..."))
	    openAction.shortcut = Qt::KeySequence.new(tr("Ctrl+O"))
	
	    @printAction = fileMenu.addAction(tr("&Print..."))
	    @printAction.enabled = false
	    @printAction.shortcut = Qt::KeySequence.new(tr("Ctrl+P"))
	
	    quitAction = fileMenu.addAction(tr("E&xit"))
	    quitAction.shortcut = Qt::KeySequence.new(tr("Ctrl+Qt::"))
	
	    helpMenu = Qt::Menu.new(tr("&Help"), self)
	    aboutAction = helpMenu.addAction(tr("&About"))
	
	    menuBar().addMenu(fileMenu)
	    menuBar().addSeparator
	    menuBar().addMenu(helpMenu)
	
	    connect(openAction, SIGNAL('triggered()'), self, SLOT('chooseImage()'))
	    connect(@printAction, SIGNAL('triggered()'), self, SLOT('printImage()'))
	    connect(quitAction, SIGNAL('triggered()'), $qApp, SLOT('quit()'))
	    connect(aboutAction, SIGNAL('triggered()'), self, SLOT('showAboutBox()'))
	
	    setWindowTitle(tr("Pixelator"))
	    resize(640, 480)
	end
	
	def chooseImage()
	    fileName = Qt::FileDialog.getOpenFileName(self,
	        tr("Choose an image"), @currentPath, "*")
	
	    if !fileName.nil?
	        if openImage(fileName)
	            @currentPath = fileName
			end
	    end
	end
	
	def openImage(fileName)
	    image = Qt::Image.new
	
	    if image.load(fileName)
	        newModel = ImageModel.new(image)
	        @view.model = newModel
	        @model = newModel
	        setWindowTitle(tr("%s - %s" % [fileName, tr("Pixelator")]))
	
	        @printAction.enabled = true
	
	        rows = @model.rowCount(Qt::ModelIndex.new)
	        columns = @model.columnCount(Qt::ModelIndex.new)
			(0...rows).each do |row|
	            @view.resizeRowToContents(row)
			end
			(0...columns).each do |column|
	            @view.resizeColumnToContents(column)
			end
	
	        return true
	    end
	    return false
	end
	
	def printImage()
	    if @model.rowCount(Qt::ModelIndex.new())*@model.columnCount(Qt::ModelIndex.new()) > 90000
	        answer = Qt::MessageBox::question(self, tr("Large Image Size"),
	            tr("The printed image may be very large. Are you sure that " +
	               "you want to print it?"),
	            Qt::MessageBox::Yes, Qt::MessageBox::No)
	        if answer == Qt::MessageBox::No
	            return
			end
	    end
	
	    printer = Qt::Printer.new(Qt::Printer::HighResolution)
	
	    dlg = Qt::PrintDialog.new(printer, self)
	    dlg.windowTitle = tr("Print Image")
	
	    if dlg.exec != Qt::Dialog::Accepted
	        return
		end
	
	    painter = Qt::Painter.new
	    painter.begin(printer)
	
	    rows = @model.rowCount(Qt::ModelIndex.new)
	    columns = @model.columnCount(Qt::ModelIndex.new)
	    sourceWidth = (columns+1) * PixelDelegate::ItemSize
	    sourceHeight = (rows+1) * PixelDelegate::ItemSize
	
	    painter.save
	
	    xscale = printer.pageRect.width/sourceWidth.to_f
	    yscale = printer.pageRect.height/sourceHeight.to_f
	    scale = [xscale, yscale].min
	
	    painter.translate(printer.paperRect.x + printer.pageRect.width/2,
	                      printer.paperRect.y + printer.pageRect.height/2)
	    painter.scale(scale, scale)
	    painter.translate(-sourceWidth/2, -sourceHeight/2)
	
	    option = Qt::StyleOptionViewItem.new
	    parent = Qt::ModelIndex.new
	
	    progress = Qt::ProgressDialog.new(tr("Printing..."), tr("Cancel"), 0, rows, self)
	    y = PixelDelegate::ItemSize/2
	
		(0...rows).each do |row|
	        progress.value = row
	        $qApp.processEvents
	        if progress.wasCanceled
	            break
			end
	
	        x = PixelDelegate::ItemSize/2
	
			(0...columns).each do |column|
	            option.rect = Qt::Rect.new(x.to_i, y.to_i, PixelDelegate::ItemSize, PixelDelegate::ItemSize)
	            @view.itemDelegate().paint(painter, option,
	                                        @model.index(row, column, parent))
	            x += PixelDelegate::ItemSize
	        end
	        y += PixelDelegate::ItemSize
	    end
	    progress.value = rows
	
	    painter.restore
	    painter.end
	
	    if progress.wasCanceled()
	        Qt::MessageBox.information(self, tr("Printing canceled"),
	            tr("The printing process was canceled."), Qt::MessageBox::Cancel)
	    end
	end
	
	def showAboutBox()
	    Qt::MessageBox.about(self, tr("About the Pixelator example"),
	        tr("This example demonstrates how a standard view and a custom\n" +
	           "delegate can be used to produce a specialized representation\n " +
	           "of data in a simple custom model."))
	end
end
