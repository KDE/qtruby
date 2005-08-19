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

end
