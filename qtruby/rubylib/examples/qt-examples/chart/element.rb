class Element

    FIELD_SEP = ':'
    PROPOINT_SEP = ';'
    XY_SEP = ','
    
    EPSILON = 0.0000001
    
    INVALID = -1
    NO_PROPORTION = -1
    MAX_PROPOINTS = 3  # One proportional point per chart type
    
    attr_accessor :value, :valueColor, :valuePattern, :label, :labelColor

    def initialize( value = INVALID, valueColor = Qt::gray,
            valuePattern = Qt::SolidPattern,
            label = nil,
            labelColor = Qt::black )
        init( value, valueColor, valuePattern, label, labelColor )
        @propoints = []
        for i in 0...MAX_PROPOINTS * 2
            @propoints[i] = NO_PROPORTION
        end
    end
    
    def isValid() return @value > EPSILON end
    
        
    def init( value, valueColor, valuePattern,
                        label, labelColor )
        @value = value
        @valueColor = valueColor
        if  Qt::SolidPattern >= valuePattern || Qt::DiagCrossPattern <= valuePattern
            valuePattern = Qt::SolidPattern
        end
        @valuePattern = valuePattern
        @label = label
        @labelColor = labelColor
    end
    
    def set( value = INVALID, valueColor = Qt::gray,
            valuePattern = Qt::SolidPattern,
            label = nil,
            labelColor = Qt::black )
        init( value, valueColor, valuePattern, label, labelColor )
    end
    
    def setValuePattern( valuePattern )
        if  valuePattern < Qt::SolidPattern.to_i || valuePattern > Qt::DiagCrossPattern.to_i
            valuePattern = Qt::SolidPattern
        end
        @valuePattern = valuePattern
    end
    
    
    def proX( index )
        return @propoints[2 * index]
    end
    
    
    def proY( index )
        return @propoints[(2 * index) + 1]
    end
    
    
    def setProX( index, value )
        @propoints[2 * index] = value
    end
    
    
    def setProY( index, value )
        @propoints[(2 * index) + 1] = value
    end
    
end

class Qt::TextStream

    alias op_write <<
	
    def <<( item )
		if !item.kind_of? Element
			return op_write(item)
		end
		element = item
        self << element.value() << Element::FIELD_SEP <<
        element.valueColor().name() << Element::FIELD_SEP << 
        element.valuePattern().to_i << Element::FIELD_SEP <<
        element.labelColor().name() << Element::FIELD_SEP
    
        for i in 0...Element::MAX_PROPOINTS
            self << element.proX( i ) << Element::XY_SEP << element.proY( i )
            self << ( i == Element::MAX_PROPOINTS - 1 ? Element::FIELD_SEP : Element::PROPOINT_SEP )
        end
    
        self << element.label() << "\n"
    
        return self
    end
    
    alias op_read >>
    
	def >>( item )
		if !item.kind_of? Element
			return op_read(item)
		end
		
		element = item
        data = readLine()
        element.value = Element::INVALID
    
        errors = 0
    
        fields = data.split( Element::FIELD_SEP )
        if  fields.length() >= 4 
            value = fields[0].to_f
            if  value.nil?
                errors += 1
            end
            valueColor = Qt::Color.new( fields[1] )
            if  !valueColor.isValid()
                errors += 1
            end
            valuePattern = fields[2].to_i
            if  valuePattern.nil?
                errors += 1
            end
            labelColor = Qt::Color.new( fields[3] )
            if  !labelColor.isValid()
                errors += 1
            end
            propoints = fields[4].split( Element::PROPOINT_SEP )
            label = data.split(Element::FIELD_SEP)[5]
            if errors == 0
                element.set( value, valueColor, valuePattern, label, labelColor )
                i = 0
                propoints.each do |point|
                errors = 0
                    xy = point.split( Element::XY_SEP )
                    x = xy[0].to_f
                    if  x.nil? || x <= 0.0 || x >= 1.0
                        errors += 1
                    end
                    y = xy[1].to_f
                    if  y.nil? || y <= 0.0 || y >= 1.0
                        errors += 1
                    end
                    if  errors > 0
                        x = y = Element::NO_PROPORTION
                    end
                    element.setProX( i, x )
                    element.setProY( i, y )
                    i += 1
                end
            end
        end
    
        return self
    end

end
