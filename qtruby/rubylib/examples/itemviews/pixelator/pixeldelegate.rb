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
	
class PixelDelegate < Qt::AbstractItemDelegate

	ItemSize = 256
	PixelSize = 12
	
	def initialize(parent = nil)
		super(parent)
	end

	def paint(painter, option, index)
	    painter.renderHint = Qt::Painter::Antialiasing
	    painter.brush = Qt::Brush.new(Qt::white)
	    painter.pen = Qt::NoPen
	    painter.drawRect(option.rect)
	    painter.brush = Qt::Brush.new(Qt::black)
	
	    size = [option.rect.width(), option.rect.height()].min
	    brightness = index.model().data(index, Qt::DisplayRole).to_i
	    radius = (size/2.0) - (brightness/255.0 * size/2.0)
	
	    painter.save
	    painter.translate(option.rect.x() + option.rect.width()/2 - radius,
	                       option.rect.y() + option.rect.height()/2 - radius)
	    painter.drawEllipse(Qt::RectF.new(0.0, 0.0, 2*radius, 2*radius))
	    painter.restore
	end
	
	def sizeHint(option, index)
	    return Qt::Size.new(PixelSize, PixelSize)
	end
end
