require 'qtruby'
require 'test/unit'

class TestQtRuby < Test::Unit::TestCase

  def test_link_against_qt4
    app = Qt::Application.new(ARGV)
    assert app
    assert_raise(NoMethodError) { app.setMainWidget(nil) }
  end

end
