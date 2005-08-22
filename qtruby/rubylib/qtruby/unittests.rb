require 'Qt'
require 'test/unit'

class TestQtRuby < Test::Unit::TestCase

  def setup
    @app = Qt::Application.instance || Qt::Application.new(ARGV)
    assert @app
  end

  def test_link_against_qt4
    assert_raise(NoMethodError) { @app.setMainWidget(nil) }
  end

  def test_qapplication_methods
   assert @app == Qt::Application::instance
   assert @app == Qt::CoreApplication::instance
   assert @app == Qt::Application.instance
   assert @app == Qt::CoreApplication.instance
   assert @app == $qApp
  end

  def test_qapplication_inheritance
   assert @app.inherits("Qt::Application")
   assert @app.inherits("Qt::CoreApplication")
   assert @app.inherits("Qt::Object")
  end

  def test_widget_inheritance
    widget = Qt::Widget.new(nil)
    assert widget.inherits("Qt::Widget")
    assert widget.inherits("Qt::Object")
    assert widget.inherits("QObject")
  end

  def test_qstring_marshall
    widget = Qt::Widget.new(nil)
    assert widget.objectName.nil?
    widget.objectName = "Barney"
    assert widget.objectName == "Barney"
  end

  def test_widgetlist
    w1 = Qt::Widget.new(nil)
    w2 = Qt::Widget.new(w1)
    w3 = Qt::Widget.new(w1)
    w4 = Qt::Widget.new(w2)

    assert w1.children == [ w2, w3 ]
  end

  def test_find_children
    w = Qt::Widget.new(nil)
    assert_raise(TypeError) { w.findChildren(nil) }

    assert w.findChildren(Qt::Widget) == [ ]
    w2 = Qt::Widget.new(w)

    assert w.findChildren(Qt::Widget) == [ w2 ]
    assert w.findChildren(Qt::Object) == [ w2 ]
    assert w.findChildren(Qt::LineEdit) == [ ]
    assert w.findChildren(Qt::Widget,"Bob") == [ ]
    assert w.findChildren(Qt::Object,"Bob") == [ ]

    w2.objectName = "Bob"

    assert w.findChildren(Qt::Widget) == [ w2 ]
    assert w.findChildren(Qt::Object) == [ w2 ]
    assert w.findChildren(Qt::Widget,"Bob") == [ w2 ]
    assert w.findChildren(Qt::Object,"Bob") == [ w2 ]
    assert w.findChildren(Qt::LineEdit, "Bob") == [ ]

    w3 = Qt::Widget.new(w)
    w4 = Qt::LineEdit.new(w2)
    w4.setObjectName("Bob")

    assert w.findChildren(Qt::Widget) == [ w4, w2, w3 ]
    assert w.findChildren(Qt::LineEdit) == [ w4 ]
    assert w.findChildren(Qt::Widget,"Bob") == [ w4, w2 ]    
    assert w.findChildren(Qt::LineEdit,"Bob") == [ w4 ]    
  end

#  def test_find_child
#    w = Qt::Widget.new(nil)
#    assert_raise(ArgumentError) { w.findChild(nil) }
#
#    assert_nil w.findChild(Qt::Widget)
#    w2 = Qt::Widget.new(w)

#    w3 = Qt::Widget.new(w)
#    w3.objectName = "Bob"
#    w4 = Qt::LineEdit.new(w2)
#    w4.objectName = "Bob"

#    assert w.findChild(Qt::Widget,"Bob") == w3
#    assert w.findChild(Qt::LineEdit,"Bob") == w4
#  end

end
