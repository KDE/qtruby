def bug1
   p1 = Qt::Point.new(5,5)
   p3 = p1.dup
   p3.setX 5
   p p1
   p p3
end
#bug1
