class FindForm

def find()
    emit lookfor( @findLineEdit.text() )
end

def notfound()
    @findLineEdit.selectAll()
end

end
