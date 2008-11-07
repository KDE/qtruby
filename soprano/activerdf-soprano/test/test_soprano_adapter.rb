# Author:: Eyal Oren
# Copyright:: (c) 2005-2006
# License:: LGPL
# Amended to use the Soprano adapter by Richard Dale

require 'test/unit'
require 'rubygems'
require 'active_rdf'
require 'federation/federation_manager'
require 'queryengine/query'

class TestSopranoAdapter < Test::Unit::TestCase
  def setup
    @test_service = 'org.soprano.Server'
    # @test_service = 'org.kde.NepomukServer'
    ConnectionPool.clear
  end

  def teardown
  end

  def test_registration
    adapter = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service)
    assert_instance_of SopranoAdapter, adapter
  end

  def test_soprano_connections
    adapter = SopranoAdapter.new({})
    assert_instance_of SopranoAdapter, adapter
  end

  def test_simple_query
    adapter = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service, :model => 'test_person')

    eyal = RDFS::Resource.new 'eyaloren.org'
    age = RDFS::Resource.new 'foaf:age'
    test = RDFS::Resource.new 'test'

    adapter.add(eyal, age, test)
    result = Query.new.distinct(:s).where(:s, :p, :o).execute(:flatten)

    assert_instance_of RDFS::Resource, result
    assert_equal 'eyaloren.org', result.uri

    adapter.clear
  end

  def test_federated_query
    adapter1 = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service, :model => 'test_person')
    adapter2 = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service, :model => 'test_person', :fake_symbol_to_get_unique_adapter => true)

    eyal = RDFS::Resource.new 'eyaloren.org'
    age = RDFS::Resource.new 'foaf:age'
    test = RDFS::Resource.new 'test'
    test2 = RDFS::Resource.new 'test2'

    adapter1.add(eyal, age, test)
    adapter2.add(eyal, age, test2)

    # assert only one distinct subject is found (same one in both adapters)
    assert_equal 1, Query.new.distinct(:s).where(:s, :p, :o).execute.size

    # assert two distinct objects are found
    results = Query.new.distinct(:o).where(:s, :p, :o).execute
    assert_equal 2, results.size

    results.all? {|result| assert result.instance_of?(RDFS::Resource) }

    adapter1.clear
  end

  def test_query_with_block
    adapter = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service, :model => 'test_person')

    eyal = RDFS::Resource.new 'eyaloren.org'
    age = RDFS::Resource.new 'foaf:age'
    test = RDFS::Resource.new 'test'

    adapter.add(eyal, age, test)
    Query.new.select(:s,:p).where(:s,:p,:o).execute do |s,p|
      assert_equal 'eyaloren.org', s.uri
      assert_equal 'foaf:age', p.uri
    end

    adapter.clear
  end
  
  def test_load_from_file
    adapter = ConnectionPool.add_data_source :type => :soprano, :service => @test_service, :model => 'test_person'
    # adapter.load("/tmp/test_person_data.nt", "turtle")
    # adapter.load("/home/metaman/workspaces/deri-workspace/activerdf/test/test_person_data.nt", "turtle")
    adapter.load("#{File.dirname(__FILE__)}/test_person_data.nt", "turtle")
    assert_equal 28, adapter.size

    adapter.clear
  end

  def test_person_data
    ConnectionPool.add_data_source :type => :soprano, :service => @test_service, :model => 'test_person'
    Namespace.register(:test, 'http://activerdf.org/test/')

    eyal = Namespace.lookup(:test, :eyal)
    eye = Namespace.lookup(:test, :eye)
    person = Namespace.lookup(:test, :Person)
    type = Namespace.lookup(:rdf, :type)
    resource = Namespace.lookup(:rdfs,:resource)

    color = Query.new.select(:o).where(eyal, eye,:o).execute
    assert 'blue', color
    assert_instance_of String, color

    ObjectManager.construct_classes
    assert eyal.instance_of?(TEST::Person)
    assert eyal.instance_of?(RDFS::Resource)

    adapter.clear
  end

  def test_federated_query
    adapter1 = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service, :model => 'test_person')
    adapter2 = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service, :model => 'test_person', :fake_symbol_to_get_unique_adapter => true)

    eyal = RDFS::Resource.new 'eyaloren.org'
    age = RDFS::Resource.new 'foaf:age'
    test = RDFS::Resource.new 'test'
    test2 = RDFS::Resource.new 'test2'

    adapter1.add(eyal, age, test)
    adapter2.add(eyal, age, test2)

    # assert only one distinct subject is found (same one in both adapters)
    assert_equal 1, Query.new.distinct(:s).where(:s, :p, :o).execute.size

    # assert two distinct objects are found
    results = Query.new.distinct(:o).where(:s, :p, :o).execute
    assert_equal 2, results.size

    results.all? {|result| assert result.instance_of?(RDFS::Resource) }

    adapter1.clear
  end

  def test_query_with_block
    adapter = ConnectionPool.add_data_source(:type => :soprano, :service => @test_service, :model => 'test_person')

    eyal = RDFS::Resource.new 'eyaloren.org'
    age = RDFS::Resource.new 'foaf:age'
    test = RDFS::Resource.new 'test'

    adapter.add(eyal, age, test)
    Query.new.select(:s,:p).where(:s,:p,:o).execute do |s,p|
      assert_equal 'eyaloren.org', s.uri
      assert_equal 'foaf:age', p.uri
    end

    adapter.clear
  end

  def test_person_data
    adapter = ConnectionPool.add_data_source :type => :soprano, :service => @test_service, :model => 'test_person'
    adapter.load("#{File.dirname(__FILE__)}/test_person_data.nt", "turtle")

    Namespace.register(:test, 'http://activerdf.org/test/')

    eyal = Namespace.lookup(:test, :eyal)
    eye = Namespace.lookup(:test, :eye)
    person = Namespace.lookup(:test, :Person)
    type = Namespace.lookup(:rdf, :type)
    resource = Namespace.lookup(:rdfs,:resource)

    assert_equal 'blue', eyal.test::eye

    ObjectManager.construct_classes
    assert eyal.instance_of?(TEST::Person)
    assert eyal.instance_of?(RDFS::Resource)
    adapter.clear
  end

  def test_sparql_query
    adapter = ConnectionPool.add_data_source :type => :soprano, :service => @test_service, :model => 'test_person'

    eyal = RDFS::Resource.new 'eyaloren.org'
    age = RDFS::Resource.new 'foaf:age'
    test = RDFS::Resource.new 'test'
    adapter.add(eyal, age, test)

    adapter.save 
    query = Query.new.distinct(:s).where(:s,:p,:o)
    results = query.execute

    assert results.include?('eyaloren.org')

    adapter.clear
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
