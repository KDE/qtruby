require 'Qt'

class Qt::Point
   def to_s
      "(#{x}, #{y})"
   end
end

$t = binding
def test(str)
   puts "#{str.ljust 25} => #{eval(str, $t)}"
end

test("p1 = Qt::Point.new(5,5)")
test("p2 = Qt::Point.new(20,20)")
test("p1 + p2")
test("p1 - p2")
test("p2 += p1")
test("p2 -= p1")
test("p2 * 3")
