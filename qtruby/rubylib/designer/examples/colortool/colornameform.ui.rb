class ColorNameForm

@colors = {}

def setColors( colors )
    @colors = colors
end

def validate()
    name = @colorLineEdit.text()
    if ! name.empty? &&
     ( @colors.empty? || ! @colors.has_key?( name ) ) 
    accept()
    else
        @colorLineEdit.selectAll()
    end
end

end
