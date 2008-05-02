require 'test/unit'
require 'korundum4'

    class MyPrefs < KDE::ConfigSkeleton
      def initialize(a)
        super(a)
      end
    end


class TestRbkconfigCompiler < Test::Unit::TestCase
  def setup
    i = KDE::ComponentData.new("test")
  end

  def teardown
  end

  def test1
    system("rbkconfig_compiler4 test1.kcfg test1.kcfgc")
    assert_equal IO.read("test1.rb"), IO.read("test1.rb.ref")

    require 'test1.rb'
    t = Test1.new(nil, nil)
  end

  def test2
    system("rbkconfig_compiler4 test2.kcfg test2.kcfgc")
    assert_equal IO.read("test2.rb"), IO.read("test2.rb.ref")

    require 'test2.rb'
    t = Test2.new()
  end

  def test3
    system("rbkconfig_compiler4 test3.kcfg test3.kcfgc")
    assert_equal IO.read("test3.rb"), IO.read("test3.rb.ref")

    require 'test3.rb'
    t = TestNameSpace::Test3.new
  end

  def test4
    system("rbkconfig_compiler4 test4.kcfg test4.kcfgc")
    assert_equal IO.read("test4.rb"), IO.read("test4.rb.ref")

    require 'test4.rb'
    t = Test4.instance(nil, nil)
  end

  def test5
    system("rbkconfig_compiler4 test5.kcfg test5.kcfgc")
    assert_equal IO.read("test5.rb"), IO.read("test5.rb.ref")

    require 'test5.rb'
    t = Test5.instance(nil, nil)
  end

  def test6
    system("rbkconfig_compiler4 test6.kcfg test6.kcfgc")
    assert_equal IO.read("test6.rb"), IO.read("test6.rb.ref")

    require 'test6.rb'
    t = Test6.new(123)
  end

  def test7
    system("rbkconfig_compiler4 test7.kcfg test7.kcfgc")
    assert_equal IO.read("test7.rb"), IO.read("test7.rb.ref")

    require 'test7.rb'
    t = Test7.new(123)
  end

  def test8a
    system("rbkconfig_compiler4 test8a.kcfg test8a.kcfgc")
    assert_equal IO.read("test8a.rb"), IO.read("test8a.rb.ref")

    require 'test8a.rb'
  end

  def test8b
    system("rbkconfig_compiler4 test8b.kcfg test8b.kcfgc")
    assert_equal IO.read("test8b.rb"), IO.read("test8b.rb.ref")

    require 'test8b.rb'
    # Fails because 8a is a singleton, and 8b inherits from it
    # t = Test8b.new(nil)
    # So Test8b must be instanciated as a Singleton too
    t = Test8b.instance(nil)
  end

  def test9
    system("rbkconfig_compiler4 test9.kcfg test9.kcfgc")
    assert_equal IO.read("test9.rb"), IO.read("test9.rb.ref")

    require 'test9.rb'
    t = Test9.new(nil, nil)
  end

  def test10
    system("rbkconfig_compiler4 test10.kcfg test10.kcfgc")
    assert_equal IO.read("test10.rb"), IO.read("test10.rb.ref")

    require 'test10.rb'
    # Fails because ItemUrlList constructors isn't implemented
    #t = Test10.instance(nil, nil)
  end

  def test_signal
    system("rbkconfig_compiler4 test_signal.kcfg test_signal.kcfgc")
    assert_equal IO.read("test_signal.rb"), IO.read("test_signal.rb.ref")

    require 'test_signal.rb'
    t = TestSignal.instance(nil, nil)
  end

  def test_settings
    system("rbkconfig_compiler4 settings.kcfg settings.kcfgc")
    assert_equal IO.read("settings.rb"), IO.read("settings.rb.ref")

    require 'settings.rb'
    t = Settings.instance(nil, nil)
  end
end
