class ColorNameForm

@m_colors = {}

def setColors( colors )
    @m_colors = colors
end

def validate()
    name = @colorLineEdit.text()
    if ! name.empty? &&
	 ( @m_colors.empty? || ! @m_colors.has_key?( name ) ) 
	accept()
    else
	@colorLineEdit.selectAll()
	end
end

end
