
class CanvasText < Qt::CanvasText

    CANVAS_TEXT = 1100
    attr :index
    
    def initialize(index, *k)
        super(*k)
        @index = index
    end
    
    def rtti() return CANVAS_TEXT end

end


