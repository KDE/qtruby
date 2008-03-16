require 'active_rdf'
require 'queryengine/query2sparql'
require 'korundum4'

# Soprano DBus server adapter
class SopranoAdapter < ActiveRdfAdapter
  $activerdflog.info "loading Soprano adapter"
  ConnectionPool.register_adapter(:soprano, self)

  attr_reader :engine
  attr_reader :caching

  @@soprano_cache = {}

  def SopranoAdapter.get_cache
    return @@soprano_cache
  end
  
  # Instantiate the connection with the SPARQL Endpoint.
  # available parameters:
  # * :model => name of model to use, defaults to 'main'
  def initialize(params = {})  
    @reads = true
    @writes = false

    model = params[:model] || ''
    @caching = params[:caching] || false

    @client = Soprano::Client::DBusClient.new
    @model = @client.createModel(model)
  end

  def size
    query(Query.new.select(:s,:p,:o).where(:s,:p,:o)).size
  end

  # query datastore with query string (SPARQL), returns array with query results
  # may be called with a block
  def query(query, &block)
    qs = Query2SPARQL.translate(query)

    if @caching
       result = query_cache(qs)
       if result.nil?
         $activerdflog.debug "cache miss for query #{qs}"
       else
         $activerdflog.debug "cache hit for query #{qs}"
         return result
       end
    end

    result = execute_soprano_query(qs, query.select_clauses, &block)
    add_to_cache(qs, result) if @caching
    result = [] if result == "timeout"
    return result
  end
    
  # do the real work of executing the sparql query
  def execute_soprano_query(qs, select_clauses, &block)
    results = []

    # querying soprano server
    binding_set = @model.executeQuery(qs, Soprano::Query::QueryLanguageSparql)

    binding_set.each do |binding|
      result = []
      select_clauses.each do |var|
        if binding[var]
          result << soprano_node_to_activerdf(binding[var])
        end
      end
      results << result
    end

    if block_given?
      results.each do |*clauses|
        yield(*clauses)
      end
    else
      results
    end
  end
  
  def close
    ConnectionPool.remove_data_source(self)
  end
  
  private
  def add_to_cache(query_string, result)
    unless result.nil? or result.empty?
      if result == "timeout"
        @@soprano_cache.store(query_string, [])
      else 
        $activerdflog.debug "adding to soprano cache - query: #{query_string}"
        @@soprano_cache.store(query_string, result) 
      end
    end
  end
  
  
  def query_cache(query_string)
    if @@soprano_cache.include?(query_string)
      return @@soprano_cache.fetch(query_string)
    else
      return nil
    end
  end

  # Converts a Soprano::Node to the ActiveRDF equivalent
  def soprano_node_to_activerdf(node)
      case node.type.to_i
      when Soprano::Node::EmptyNode:
        "(empty)"
      when Soprano::Node::ResourceNode:
        RDFS::Resource.new(node.uri.toString)
      when Soprano::Node::LiteralNode:
        if node.literal.isString && !node.language.empty?
          '"%s"@%s' % [node.literal.toString, node.language]
        else 
          '"%s"^^<%s>' % [node.literal.toString, node.literal.dataTypeUri.toString]
        end
      when Soprano::Node::BlankNode:
        BNode.new("_:#{node.identifier}")
      end
  end
  
end
